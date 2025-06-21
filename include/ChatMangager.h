#pragma once
#include <QObject>
#include <QDateTime>
#include <QListWidget>
#include <QList>
#include <cstddef>
struct ChatMetadata {
    int id;
    QString title;
    QDateTime created;
    QDateTime lastModified;
};
Q_DECLARE_METATYPE(ChatMetadata)
class ChatManager : public QObject {
    Q_OBJECT

public:




    ChatManager(QListWidget* list,const QString& chatsDirectory = "../chats");
    ~ChatManager();
    void changeChats(int idToChangeTo);
    ChatMetadata addNewChatToSideBar(const QString& title = "New Chat");
    void saveChatToDisk(ChatMetadata data, const QJsonArray& messages);
    QJsonArray getChatFromDisk(const int chatId);
    void deleteChat(const int chatId);
    void updateChatTitle(const int chatId, const QString& newTitle);
    ChatMetadata getChatData(const int chatId);

private:
    QList<ChatMetadata> m_chatList;
    QListWidget* chatList;
    int m_currentId;
    QString m_chatsDirectory;
    void writeChatsToDisk();
};
