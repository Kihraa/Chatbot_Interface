#include "ChatBubble.h"
#include "LaTeXLabel.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPainter>

ChatBubble::ChatBubble(const QString &message, BubbleType type, QWidget *parent)
    : QWidget(parent), m_message(message), m_type(type) {
    setupUI();
    setStyleSheet(getBubbleStyle());
}

void ChatBubble::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(10, 8, 10, 8);

    // Message text using the chat-optimized viewer
    LaTeXLabel* messageViewer = new LaTeXLabel(this);
    messageViewer->setContent(m_message);
    messageViewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    layout->addWidget(messageViewer);

    // Set maximum width so long messages wrap
    if(m_type == BubbleType::Sent) {
        setMaximumWidth(500);
        messageViewer->setMaximumWidth(480); // Account for margins
    } else {

    }

    // Allow the bubble to grow with content
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
}

QString ChatBubble::getBubbleStyle() {
    if (m_type == Sent) {
        return R"(
                background-color: #0084ff;
                color: white;
                border-radius: 18px;
                font-size: 25px;
        )";
    } else {
        return R"(
                background-color: #e4e6ea;
                color: black;
                border-radius: 18px;
                font-size: 25px;
        )";
    }
}
