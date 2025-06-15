#ifndef LaTeXLabel_H
#define LaTeXLabel_H

#include <QLabel>
#include <QTextBrowser>
#include <QSvgRenderer>
#include <QPainter>
#include <QRegularExpression>

class MarkdownLatexLabel : public QLabel
{
    Q_OBJECT

public:
    explicit MarkdownLatexLabel(QWidget *parent = nullptr);
    void setMarkdownLatexText(const QString &text);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct LatexExpression {
        QString latex;
        bool displayMode;
        QRect boundingRect;
        QString svgData;
    };

    QString m_originalText;
    QString m_htmlText;
    QList<LatexExpression> m_latexExpressions;

    QString markdownToHtml(const QString &markdown);
    QString extractLatexFromHtml(const QString &html, QList<LatexExpression> &expressions);
    QString latexToSvg(const QString &latex, bool displayMode);
    void updateLatexPositions();
    void calculatePositionFromText(const QString &textBefore, const QFontMetrics &fm, int labelWidth, double &posX, double &posY);
    QString extractTextBeforePlaceholder(const QString &html, const QString &placeholderId);
};

class LaTeXLabel : public QTextBrowser
{
    Q_OBJECT

public:
    explicit LaTeXLabel(QWidget *parent = nullptr);
    void setContent(const QString &markdownLatexText);

private:
    MarkdownLatexLabel *m_label;
};

#endif // LaTeXLabel_H
