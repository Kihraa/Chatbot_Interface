#include "LaTeXLabel.h"
#include <QRegularExpression>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <md4c-html.h>

int fontSize=25;

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
    setMargin(10);
    setStyleSheet("border: 1px black; background-color: white; color: black;");
    setWordWrap(true);
    setTextFormat(Qt::RichText);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    setTextInteractionFlags(Qt::TextSelectableByMouse);

    setAlignment(Qt::AlignTop | Qt::AlignLeft);

}

void MarkdownLatexLabel::setMarkdownLatexText(const QString &text)
{
    m_originalText = text;

    // Step 1: Convert Markdown to HTML
    QString htmlText = markdownToHtml(text);

    // Step 2: Extract LaTeX expressions and replace with PNG images
    m_htmlText = extractLatexFromHtml(htmlText, m_latexExpressions);

    // Step 3: Wrap content with font size styling
    m_htmlText = QString("<div style=\"font-size: %1px;\">%2</div>").arg(fontSize).arg(m_htmlText);

    // Step 4: Set the HTML content to the label
    setText(m_htmlText);
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

        // Create expression entry and generate PNG
        LatexExpression expr;
        expr.latex = latexCode;
        expr.displayMode = isDisplayMode;

        // Generate PNG for this LaTeX expression
        int height,width;

        QString pngBase64 = latexToPngBase64(latexCode, isDisplayMode,height,width);
        QString imgTag;
        int inlineHeight=100;
        if (!pngBase64.isEmpty()) {
            QString style;
            if (isDisplayMode) {
                // Display math: left-aligned with size control
                style = QString("display: block; margin: 10px; text-align: left; "
                       "max-width: 100%; max-height: %2; "
                       "object-fit: contain;").arg(inlineHeight);
            } else { // inline
                style = QString("display: inline; vertical-align: middle; "
                       "max-width: %1; max-height: %2px; "
                       "object-fit: contain;").arg(width*(fontSize/(double)height)).arg(fontSize);
            }

            imgTag = QString("<img src=\"data:image/png;base64,%1\" style=\"%2\" alt=\"LaTeX: %3\" />")
                    .arg(pngBase64)
                    .arg(style)
                    .arg(QString(latexCode).replace("\"", "&quot;"));
        }


        expressions.append(expr);

        // Replace the LaTeX expression with the img tag
        result.replace(match.capturedStart() + offset, match.capturedLength(), imgTag);
        offset += imgTag.length() - match.capturedLength();
        expressionIndex++;
    }

    return result;
}

QString MarkdownLatexLabel::latexToPngBase64(const QString &latex, bool displayMode, int& height, int& width)
{
    QString tempDir = QDir::tempPath();
    QString baseName = QString("latex_%1").arg(QDateTime::currentMSecsSinceEpoch());
    QString texFile = QString("%1/%2.tex").arg(tempDir, baseName);
    QString pngFile = QString("%1/%2.png").arg(tempDir, baseName);

    // Create LaTeX file
    QFile file(texFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return QString();
    }

    QTextStream out(&file);
    out << "\\documentclass[12pt]{article}\n";
    out << "\\usepackage{amsmath,amsfonts,amssymb}\n";
    out << "\\usepackage[active,tightpage]{preview}\n";
    out << "\\usepackage{xcolor}\n";
    out << "\\pagestyle{empty}\n";

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


    // Compile LaTeX -> DVI
    QProcess latexProcess;
    latexProcess.setWorkingDirectory(tempDir);
    latexProcess.start("latex", QStringList() << "-interaction=nonstopmode" << texFile);
    latexProcess.waitForFinished(5000);

    if (latexProcess.exitCode() != 0) {
        qDebug() << "LaTeX compilation failed:" << latexProcess.readAllStandardError();
        cleanup(tempDir, baseName);
        return QString();
    }

    // Convert DVI -> PNG using dvipng with verbose output
    QString dviFile = QString("%1/%2.dvi").arg(tempDir, baseName);
    QProcess dvipngProcess;
    QStringList dvipngArgs;
    dvipngArgs << "-D" << "300"
               << "-T" << "tight"
               << "-bg" << "Transparent"
               << "-Q" << "5"
               << "--height"
               << "--width"
               << "-o" << pngFile
               << dviFile;

    dvipngProcess.start("dvipng", dvipngArgs);
    dvipngProcess.waitForFinished(5000);

    if (dvipngProcess.exitCode() != 0) {
        qDebug() << "dvipng conversion failed:" << dvipngProcess.readAllStandardError();
        cleanup(tempDir, baseName);
        return QString();
    }

    // Parse dvipng output to get dimensions
    QString dvipngOutput = dvipngProcess.readAllStandardOutput();



    // dvipng outputs something like: "This is dvipng 1.17 ... [1 depth=0 height=13 width=42]"
    QRegularExpression dimRegex(R"(\[1.*?height=(\d+).*?width=(\d+)\])");
    QRegularExpressionMatch dimMatch = dimRegex.match(dvipngOutput);

    if (dimMatch.hasMatch()) {
        height = dimMatch.captured(1).toInt();
        width = dimMatch.captured(2).toInt();
    }

    // Read PNG file and convert to base64
    QFile pngFileHandle(pngFile);
    QString base64Data;

    if (pngFileHandle.open(QIODevice::ReadOnly)) {
        QByteArray pngData = pngFileHandle.readAll();
        base64Data = pngData.toBase64();
    }

    // Cleanup temp files
    cleanup(tempDir, baseName);

    return base64Data;
}


void MarkdownLatexLabel::cleanup(const QString &tempDir, const QString &baseName)
{
    // Remove all temporary files
    QStringList extensions = {"tex", "dvi", "png", "aux", "log"};
    for (const QString &ext : extensions) {
        QString fileName = QString("%1/%2.%3").arg(tempDir, baseName, ext);
        QFile::remove(fileName);
    }
}
