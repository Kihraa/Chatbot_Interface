#pragma once

#include <QWidget>

class QLabel;

class ChatBubble : public QWidget {
    Q_OBJECT

public:
    enum BubbleType { Sent, Received };

    ChatBubble(const QString &message, BubbleType type, QWidget* parent = nullptr);

private:
    QString m_message;
    BubbleType m_type;

    void setupUI();
    QString getBubbleStyle();
};
