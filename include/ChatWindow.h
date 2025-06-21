#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QJsonArray>
#include "ChatMangager.h"
class ChatWindow : public QWidget {
    Q_OBJECT

public:
    ChatWindow(ChatManager* manager,QWidget* parent = nullptr);
    enum BubbleType { Sent, Received };
    QJsonArray context;
    ChatMetadata metadata;
    ChatManager* manager;
    void fillChat(QJsonArray& messages);
    void clearAndDisplayChat(ChatMetadata chat);


public slots:
    void addMessage(const QString &message, BubbleType type);

private:
    QScrollArea* m_scrollArea;    // Provides scrolling capability
    QVBoxLayout* m_chatLayout;    // Main layout where messages are added
    QLineEdit* m_messageInput;    // Text input field
    QComboBox* m_modelSelector;
    QList<QLayout*> m_messageLayouts;
    void clearMessages();



private slots:
    void sendMessage();
    void newChat();
};
