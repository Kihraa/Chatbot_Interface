#include "ChatMangager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QJsonObject>


ChatManager::ChatManager(QListWidget* list,const QString& chatsDirectory) : chatList(list){
    m_currentId=0;
    QFile key_file("../chats/chat_list.json");
    //create if it doesn't exist
    if (!key_file.exists()) {
        int status =key_file.open(QIODevice::WriteOnly | QIODevice::Text);
        if (!status) {
            qWarning() << "Failed to create file:" << key_file.errorString();
            return;
        }

        key_file.write("[]");
        key_file.close();
    }

    //open
    if (!key_file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << key_file.errorString();
        return;
    }
    QString contents = key_file.readAll();
    key_file.close();
    QJsonDocument doc = QJsonDocument::fromJson(contents.toUtf8());
    if (!doc.isArray()) {
        qWarning() << "Expected top-level JSON array";
        return;
    }

    for(const QJsonValue& elem : doc.array()){
        ChatMetadata chat;
        chat.id =elem["id"].toInt();
        m_currentId=fmax(m_currentId,chat.id);
        chat.title=elem["title"].toString();
        chat.created=QDateTime::fromString(elem["created"].toString());
        chat.lastModified=QDateTime::fromString(elem["last_modified"].toString());
        QListWidgetItem* item = new QListWidgetItem(chat.title);
        item->setData(Qt::UserRole,QVariant::fromValue(chat));
        chatList->addItem(item);
    }
}

ChatManager::~ChatManager(){
    //save chat list to chat_list.json
    writeChatsToDisk();
}

ChatMetadata ChatManager::addNewChatToSideBar(const QString& title){
    ChatMetadata chat ={
        m_currentId++,
        title,
        QDateTime::currentDateTime(),
        QDateTime::currentDateTime()};

    QListWidgetItem* item = new QListWidgetItem(chat.title);
    item->setData(Qt::UserRole,QVariant::fromValue(chat));
    chatList->addItem(item);
    return chat;
}
void ChatManager::saveChatToDisk(ChatMetadata meta, const QJsonArray& messages) {
    // Create JSON object
    QJsonObject chat;
    chat["id"] = meta.id;
    chat["title"] = meta.title;
    chat["created"] = meta.created.toString();
    chat["lastModified"] = meta.lastModified.toString();
    chat["messages"] = messages;
    QJsonDocument doc(chat);

    // Open file for writing (this will create it if it doesn't exist)
    QFile chat_file(QString("../chats/chat_%1.json").arg(meta.id));
    if (!chat_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << chat_file.errorString();
        return;
    }

    // Write JSON and close file
    chat_file.write(doc.toJson());
    chat_file.close(); // This is the missing line!
}


QJsonArray ChatManager::getChatFromDisk(const int chatId){

    QFile chat_file(QString("../chats/chat_%1.json").arg(chatId));

    //open
    if (!chat_file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << chat_file.errorString();
        QJsonArray errorArr=QJsonArray();
        return errorArr;
    }
    QString contents = chat_file.readAll();
    chat_file.close();
    QJsonDocument doc = QJsonDocument::fromJson(contents.toUtf8());
    QJsonValue messageArray= doc["messages"];
    return messageArray.toArray();
}

void ChatManager::deleteChat(const int chatId){
    // remove label from ui
    int n= chatList->count();
    for (int i=0; i<n; i++) {
        if(chatList->item(i)->data(Qt::UserRole).toInt()==chatId){
            QListWidgetItem* item = chatList->takeItem(i);
            delete item;
            break;
        }
    }

    // remove file chat_id.json
    if (QFile::remove(QString("../chats/chat_%1.json").arg(chatId))) {
        qDebug() << "File deleted successfully.";
    } else {
        qDebug() << "Failed to delete file.";
    }

    // chat is removed from chat_lists.json when app is closed
}
void ChatManager::updateChatTitle(const int chatId, const QString& newTitle){
    for (int i = 0; i < chatList->count(); i++) {
        ChatMetadata meta =chatList->item(i)->data(Qt::UserRole).value<ChatMetadata>();
        if (meta.id == chatId) {
            chatList->item(i)->setText(newTitle);
            meta.title=newTitle;
            chatList->item(i)->setData(Qt::UserRole, QVariant::fromValue(meta));
            break;
        }
    }
}
void ChatManager::writeChatsToDisk(){
    QJsonArray array;
    for (int i = 0; i < chatList->count(); i++) {
        ChatMetadata chat =chatList->item(i)->data(Qt::UserRole).value<ChatMetadata>();
        QJsonObject obj;
        obj["id"] = chat.id;
        obj["title"] = chat.title;
        obj["created"] = chat.created.toString();
        obj["last_modified"] = chat.lastModified.toString();
        array.append(obj);
    }

    QFile file("../chats/chat_list.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonDocument doc(array);
        file.write(doc.toJson());
        file.close();
    }
}
