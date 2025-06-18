#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QComboBox>

class ChatWindow : public QWidget {
    Q_OBJECT

public:
    ChatWindow(QWidget* parent = nullptr);
    enum BubbleType { Sent, Received };

public slots:
    void addMessage(const QString &message, BubbleType type);

private:
    QScrollArea* m_scrollArea;    // Provides scrolling capability
    QVBoxLayout* m_chatLayout;    // Main layout where messages are added
    QLineEdit* m_messageInput;    // Text input field
    QComboBox* m_modelSelector;

private slots:
    void sendMessage();
};
