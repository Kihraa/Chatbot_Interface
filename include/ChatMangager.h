#include <QObject>
#include <QDateTime>

class ChatManager : public QObject {
    Q_OBJECT

public:
    struct ChatMetadata {
        QString id;
        QString title;
        QDateTime created;
        QDateTime lastModified;
        int messageCount;
    };

    ChatManager(const QString& chatsDirectory = "./chats");

    QList<ChatMetadata> loadChatList();
    QString createNewChat(const QString& title = "New Chat");
    bool saveChat(const QString& chatId, const QJsonArray& messages, const QString& model);
    QJsonArray loadChatMessages(const QString& chatId);
    bool deleteChat(const QString& chatId);
    bool updateChatTitle(const QString& chatId, const QString& newTitle);

private:
    QString m_chatsDirectory;
    void ensureDirectoryExists();
    void updateMetadata();
};
