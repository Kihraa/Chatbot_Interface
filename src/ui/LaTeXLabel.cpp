#include "LaTeXLabel.h"
#include <QRegularExpression>
#include <QFontMetrics>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QDateTime>
#include <QTextDocument>
#include <QPaintEvent>
#include <QApplication>
#include <md4c-html.h>

// Static callback function for md_html
static void processOutputCallback(const MD_CHAR* text, MD_SIZE size, void* userdata)
{
    QByteArray* output = static_cast<QByteArray*>(userdata);
    output->append(text, static_cast<int>(size));
}

// MarkdownLatexLabel Implementation
MarkdownLatexLabel::MarkdownLatexLabel(QWidget *parent)
    : QLabel(parent)
{
    setWordWrap(true);
    setTextFormat(Qt::RichText);
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
}

void MarkdownLatexLabel::setMarkdownLatexText(const QString &text)
{
    m_originalText = text;

    // Step 1: Convert Markdown to HTML
    QString htmlText = markdownToHtml(text);

    // Step 2: Extract LaTeX expressions and replace with placeholders
    m_htmlText = extractLatexFromHtml(htmlText, m_latexExpressions);

    // Step 3: Generate SVG for all LaTeX expressions
    for (auto &expr : m_latexExpressions) {
        expr.svgData = latexToSvg(expr.latex, expr.displayMode);
    }

    // Step 4: Set the HTML content to the label
    setText(m_htmlText);

    // Step 5: Update positions for LaTeX rendering
    updateLatexPositions();
}

QString MarkdownLatexLabel::markdownToHtml(const QString &markdown)
{
    QByteArray input = markdown.toUtf8();
    QByteArray output;

    // Convert with LaTeX math support
    unsigned flags = MD_FLAG_LATEXMATHSPANS |
                    MD_FLAG_TABLES |
                    MD_FLAG_STRIKETHROUGH |
                    MD_FLAG_TASKLISTS;

    int result = md_html(
        input.constData(),
        static_cast<MD_SIZE>(input.size()),
        processOutputCallback,
        &output,
        flags,
        0
    );

    if (result != 0) {
        return QString("Error parsing Markdown");
    }

    return QString::fromUtf8(output);
}

QString MarkdownLatexLabel::extractLatexFromHtml(const QString &html, QList<LatexExpression> &expressions)
{
    QString result = html;
    expressions.clear();

    // Match <x-equation type="display">$$...$$</x-equation> and <x-equation>$...$</x-equation>
    QRegularExpression equationRegex(R"(<x-equation(?:\s+type="display")?>(.*?)</x-equation>)");
    QRegularExpressionMatchIterator matches = equationRegex.globalMatch(html);

    int offset = 0;
    int expressionIndex = 0;

    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();

        QString fullMatch = match.captured(0);
        QString latexWithDelimiters = match.captured(1);

        // Determine if it's display mode and extract LaTeX code
        bool isDisplayMode = fullMatch.contains(R"(type="display")");
        QString latexCode;

        if (isDisplayMode) {
            // Remove $$ delimiters
            if (latexWithDelimiters.startsWith("$$") && latexWithDelimiters.endsWith("$$")) {
                latexCode = latexWithDelimiters.mid(2, latexWithDelimiters.length() - 4);
            } else {
                latexCode = latexWithDelimiters;
            }
        } else {
            // Remove $ delimiters
            if (latexWithDelimiters.startsWith("$") && latexWithDelimiters.endsWith("$")) {
                latexCode = latexWithDelimiters.mid(1, latexWithDelimiters.length() - 2);
            } else {
                latexCode = latexWithDelimiters;
            }
        }

        // Create expression entry
        LatexExpression expr;
        expr.latex = latexCode;
        expr.displayMode = isDisplayMode;
        expressions.append(expr);

        // Replace with a placeholder span that we can find later
        QString placeholder = QString("<span id=\"latex_%1\" style=\"color: transparent;\">%2</span>")
                             .arg(expressionIndex)
                             .arg(isDisplayMode ? "DISPLAY_MATH" : "INLINE_MATH");


        result.replace(match.capturedStart() + offset, match.capturedLength(), placeholder);
        offset += placeholder.length() - match.capturedLength();
        expressionIndex++;
    }

    return result;
}

void MarkdownLatexLabel::updateLatexPositions()
{
    // Get the label's font metrics once
    QFontMetrics fm(this->font());
    int lineHeight = fm.height();

    for (int i = 0; i < m_latexExpressions.size(); ++i) {
        double width, height, posX, posY;

        // Get width & height of SVG (keep your existing code)
        QRegularExpression widthRe(R"(width=['"]([0-9]*\.?[0-9]+)pt['"])");
        QRegularExpression heightRe(R"(height=['"]([0-9]*\.?[0-9]+)pt['"])");
        QRegularExpressionMatch wMatch = widthRe.match(m_latexExpressions[i].svgData);
        QRegularExpressionMatch hMatch = heightRe.match(m_latexExpressions[i].svgData);

        if (wMatch.hasMatch() && hMatch.hasMatch()) {
            width = wMatch.captured(1).toDouble();
            height = hMatch.captured(1).toDouble();
        } else {
            width = 100;
            height = 20;
        }

        // Find the placeholder in the text
        QString placeholderTag = QString("latex_%1").arg(i);
        QString textBeforePlaceholder = extractTextBeforePlaceholder(this->text(), placeholderTag);

        if (textBeforePlaceholder.isEmpty()) {
            posX = 0;
            posY = i * lineHeight;
        } else {
            // Calculate position using simplified line wrapping
            calculatePositionFromText(textBeforePlaceholder, fm, this->width(), posX, posY);
        }

        qDebug() << "LaTeX" << i << "position: (" << posX << "," << posY << ")";

        m_latexExpressions[i].boundingRect = QRect(posX, posY, width, height);
    }
}

QString MarkdownLatexLabel::extractTextBeforePlaceholder(const QString &html, const QString &placeholderId)
{
    // Find the placeholder span
    QString searchPattern = QString("id=\"%1\"").arg(placeholderId);
    int placeholderPos = html.indexOf(searchPattern);

    if (placeholderPos < 0) {
        return QString();
    }

    // Extract everything before the placeholder and strip HTML tags
    QString beforePlaceholder = html.left(placeholderPos);

    // Simple HTML tag removal (you might want to use a more sophisticated approach)
    QRegularExpression htmlTagRegex("<[^>]*>");
    QString plainText = beforePlaceholder.remove(htmlTagRegex);

    // Decode HTML entities
    QTextDocument tempDoc;
    tempDoc.setHtml(plainText);
    return tempDoc.toPlainText();
}

void MarkdownLatexLabel::calculatePositionFromText(const QString &textBefore,
                                                   const QFontMetrics &fm,
                                                   int labelWidth,
                                                   double &posX,
                                                   double &posY)
{
    // Split text into words for simple word wrapping simulation
    QStringList words = textBefore.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

    int currentLineWidth = 0;
    int lineNumber = 0;

    for (const QString &word : words) {
        int wordWidth = fm.horizontalAdvance(word + " ");

        // Check if word fits on current line
        if (currentLineWidth + wordWidth > labelWidth && currentLineWidth > 0) {
            // Move to next line
            lineNumber++;
            currentLineWidth = wordWidth;
        } else {
            currentLineWidth += wordWidth;
        }
    }

    posX = currentLineWidth;
    posY = lineNumber * fm.height() + fm.ascent();
}

void MarkdownLatexLabel::paintEvent(QPaintEvent *event)
{
    // First, paint the normal HTML content
    QLabel::paintEvent(event);

    // Then, paint the LaTeX expressions on top
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (const auto &expr : m_latexExpressions) {
        if (!expr.svgData.isEmpty()) {
            QSvgRenderer renderer(expr.svgData.toUtf8());
            renderer.render(&painter, expr.boundingRect);
        }
    }
}

QString MarkdownLatexLabel::latexToSvg(const QString &latex, bool displayMode)
{
    QString tempDir = QDir::tempPath();
    QString baseName = QString("latex_%1").arg(QDateTime::currentMSecsSinceEpoch());
    QString texFile = QString("%1/%2.tex").arg(tempDir, baseName);
    QString svgFile = QString("%1/%2.svg").arg(tempDir, baseName);

    QFile file(texFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return QString();
    }

    QTextStream out(&file);
    out << "\\documentclass[12pt]{article}\n";
    out << "\\usepackage{amsmath,amsfonts,amssymb}\n";
    out << "\\usepackage[active,tightpage]{preview}\n";

    if (displayMode) {
        out << "\\PreviewEnvironment{displaymath}\n";
        out << "\\begin{document}\n";
        out << "\\begin{displaymath}\n";
        out << latex << "\n";
        out << "\\end{displaymath}\n";
    } else {
        out << "\\PreviewEnvironment{math}\n";
        out << "\\begin{document}\n";
        out << "\\begin{math}\n";
        out << latex << "\n";
        out << "\\end{math}\n";
    }

    out << "\\end{document}\n";
    file.close();

    // Compile LaTeX -> DVI -> SVG
    QProcess latexProcess;
    latexProcess.setWorkingDirectory(tempDir);
    latexProcess.start("latex", QStringList() << "-interaction=nonstopmode" << texFile);
    latexProcess.waitForFinished();

    if (latexProcess.exitCode() != 0) {
        return QString();
    }

    QString dviFile = QString("%1/%2.dvi").arg(tempDir, baseName);
    QProcess dvisvgmProcess;
    dvisvgmProcess.start("dvisvgm", QStringList() << "--no-fonts" << dviFile << "-o" << svgFile);
    dvisvgmProcess.waitForFinished();

    if (dvisvgmProcess.exitCode() != 0) {
        return QString();
    }

    QFile svgFileHandle(svgFile);
    if (svgFileHandle.open(QIODevice::ReadOnly)) {
        QString svgContent = svgFileHandle.readAll();

        // Cleanup temp files
        QFile::remove(texFile);
        QFile::remove(dviFile);
        QFile::remove(svgFile);
        QFile::remove(QString("%1/%2.aux").arg(tempDir, baseName));
        QFile::remove(QString("%1/%2.log").arg(tempDir, baseName));

        return svgContent;
    }

    return QString();
}

// LaTeXLabel Implementation (keeping for compatibility)
LaTeXLabel::LaTeXLabel(QWidget *parent)
    : QTextBrowser(parent)
{
    m_label = new MarkdownLatexLabel(this);
    // You might want to adjust the layout here
}

void LaTeXLabel::setContent(const QString &markdownLatexText)
{
    m_label->setMarkdownLatexText(markdownLatexText);
}
