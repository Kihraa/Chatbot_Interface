#include "ChatWindow.h"
#include "LaTeXLabel.h"
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QPushButton>
#include <QComboBox>


static QString defaultResponse =R"(Here's a detailed explanation of continuity of functions using only inline
LaTeX:

A function $f$ defined on an interval $I = [a,b]$ is said to be
**continuous** on $I$ if for every point $x_0 \in I$, the following
conditions are satisfied:

1.  $\displaystyle{\lim_{x \to x_0} f(x)}$ exists.
2.  $\displaystyle{\lim_{x \to x_0} f(x) = f(x_0)}.$
$$x_0 \in I$$
A function can be discontinuous in various ways, including:

*   **Jump Discontinuity**: When the left and right limits exist but are
unequal, i.e., $\displaystyle{\lim_{x \to c^-} f(x) \neq \lim_{x \to c^+}
f(x)}.$
*   **Infinite Discontinuity**: If either the left or right limit (or
both) is infinite at a point $c$, i.e., $\displaystyle{\lim_{x \to c^-}
f(x) = \pm\infty}$ or $\displaystyle{\lim_{x \to c^+} f(x) = \pm\infty}.$
*   **Removable Discontinuity**: When the limit exists at a point $c$ but
equals the function value at that point, i.e., $\displaystyle{\lim_{x \to
c^-} f(x) = \lim_{x \to c^+} f(x) = f(c)}.$

A function $f$ is said to be **continuous** if it has no discontinuities
on its entire domain. In other words, for every point in the domain of
$f$, both the left and right limits exist and are equal to the function
value at that point.

Note: The inline LaTeX syntax uses backslashes (`\`) to escape special
characters, and surrounds math expressions with dollar signs (`$`). For
example, `$\displaystyle{\lim_{x \to x_0} f(x)}$` would render as
$\displaystyle{\lim_{x \to x_0} f(x)}$.
)";

ChatWindow::ChatWindow(QWidget *parent) : QWidget(parent) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    //model selector

    QWidget* headerWidget = new QWidget;
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    QComboBox* modelSelector = new QComboBox(this);
    modelSelector->addItem("GPT-4 Turbo", "openai/gpt-4-turbo");
    modelSelector->addItem("Claude 3 Sonnet", "anthropic/claude-3-sonnet");
    modelSelector->addItem("Mistral 7B", "mistral/mistral-7b");
    modelSelector->addItem("Gemini 1.5 Pro", "google/gemini-1.5-pro");
    mainLayout->addWidget(modelSelector);
    modelSelector->setMaximumWidth(250);


    // Scrollable chat area
    m_scrollArea = new QScrollArea;
    QWidget* chatWidget = new QWidget;
    m_chatLayout = new QVBoxLayout(chatWidget);

    // Important: Add stretch at the end to push messages to top
    m_chatLayout->addStretch();

    m_scrollArea->setWidget(chatWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);



    // Input area
    QHBoxLayout* inputLayout = new QHBoxLayout;
    m_messageInput = new QLineEdit;
    QPushButton* sendButton = new QPushButton("Send");

    connect(sendButton, &QPushButton::clicked, this, &ChatWindow::sendMessage);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &ChatWindow::sendMessage);

    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(sendButton);

    mainLayout->addWidget(m_scrollArea);
    mainLayout->addLayout(inputLayout);
}

void ChatWindow::addMessage(const QString &message, ChatBubble::BubbleType type) {
    MarkdownLatexLabel* bubble = new MarkdownLatexLabel(this);
    bubble->setMarkdownLatexText(message);
    QHBoxLayout* bubbleLayout = new QHBoxLayout;

    if (type == ChatBubble::Sent) {
        bubbleLayout->addStretch();  // Push to right
        bubbleLayout->addWidget(bubble);
        bubble->setMaximumWidth(480); //account for margin
        bubble->setStyleSheet(R"(
                background-color: #0084ff;
                color: white;
                border-radius: 18px;
                font-size: 25px;
        )");

    } else {
        bubble->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        bubble->setStyleSheet(R"(
                background-color: #e4e6ea;
                color: black;
                border-radius: 18px;
                font-size: 25px;
        )");
        bubbleLayout->addWidget(bubble);

    }

    m_chatLayout->addLayout(bubbleLayout);

    // Auto-scroll to show newest message
    QTimer::singleShot(0, this, [this]() {
        m_scrollArea->verticalScrollBar()->setValue(
            m_scrollArea->verticalScrollBar()->maximum()
        );
    });
}

void ChatWindow::sendMessage() {
    QString message = m_messageInput->text().trimmed();
    if (!message.isEmpty()) {
        addMessage(message, ChatBubble::Sent);
        m_messageInput->clear();

        // Simulate received message after a delay
        QTimer::singleShot(0, this, [this]() {
            addMessage(defaultResponse, ChatBubble::Received);
        });
    }
}
