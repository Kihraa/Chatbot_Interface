#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include "ChatBubble.h"

class ChatWindow : public QWidget {
    Q_OBJECT

public:
    ChatWindow(QWidget* parent = nullptr);

public slots:
    void addMessage(const QString &message, ChatBubble::BubbleType type);

private:
    QScrollArea* m_scrollArea;    // Provides scrolling capability
    QVBoxLayout* m_chatLayout;    // Main layout where messages are added
    QLineEdit* m_messageInput;    // Text input field
    QComboBox* m_modelSelector;

private slots:
    void sendMessage();
};
