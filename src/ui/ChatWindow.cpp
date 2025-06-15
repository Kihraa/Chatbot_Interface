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

#define defaultResponse "Lorem ipsum dolor sit $amet$, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."



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
