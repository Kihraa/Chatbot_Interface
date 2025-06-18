#include "ChatWindow.h"
#include "LaTeXLabel.h"
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QPushButton>
#include <QComboBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QFile>
#include <QTimer>

QJsonArray context;

ChatWindow::ChatWindow(QWidget *parent) : QWidget(parent) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    //model selector

    QWidget* headerWidget = new QWidget;
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    m_modelSelector = new QComboBox(this);
    //placeholder models
    m_modelSelector->addItem("Claude Sonnet 4", "anthropic/claude-sonnet-4-20250514");
    m_modelSelector->addItem("Gemini 2.5 Pro", "gemini/gemini-2.5-pro-preview-05-06");
    mainLayout->addWidget(m_modelSelector);
    m_modelSelector->setMaximumWidth(250);


    // Scrollable chat area
    m_scrollArea = new QScrollArea;
    QWidget* chatWidget = new QWidget;
    m_chatLayout = new QVBoxLayout(chatWidget);

    //stretch at the end to push messages to top
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

void ChatWindow::addMessage(const QString &message, BubbleType type) {
    MarkdownLatexLabel* bubble = new MarkdownLatexLabel(this);
    bubble->setMarkdownLatexText(message);
    QHBoxLayout* bubbleLayout = new QHBoxLayout;

    if (type == Sent) {
        bubbleLayout->addStretch();  // Push to right
        bubbleLayout->addWidget(bubble);
        bubble->setMaximumWidth(480); //account for margin
        bubble->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
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
    if (message.isEmpty()) return;

    //render message sent
    addMessage(message, Sent);
    m_messageInput->clear();

    //add message to context
    QJsonObject userMsg;
    userMsg["role"]    = "user";
    userMsg["content"] = message;
    context.append(userMsg);
    QFile key_file("../api_key.txt");
    key_file.open(QIODeviceBase::ReadOnly);
    QString userkey=key_file.readLine();
    key_file.close();
    userkey.chop(1); //removes \n


    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    request.setUrl(QUrl("https://litellm.sph-prod.ethz.ch/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(userkey).toUtf8());

    QJsonObject requestBody;
    requestBody["model"] =m_modelSelector->currentData().toString();
    requestBody["messages"] = context;

    QJsonDocument doc(requestBody);
    QByteArray data = doc.toJson();

    QNetworkReply* reply = manager->post(request, data);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QString response;

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
            QJsonObject responseObj = responseDoc.object();

            QJsonArray choices = responseObj["choices"].toArray();
            if (!choices.isEmpty()) {
                QJsonObject firstChoice = choices[0].toObject();
                QJsonObject message = firstChoice["message"].toObject();
                response = message["content"].toString();

                QJsonObject assistantMsg;
                assistantMsg["role"] = "assistant";
                assistantMsg["content"] = response;
                context.append(assistantMsg);
            } else {
                response = "Sorry, I couldn't generate a response.";
            }
        } else {
            response = QString("Error: %1").arg(reply->errorString());
        }

        // display response
        addMessage(response, Received);

        reply->deleteLater();

    });

}
