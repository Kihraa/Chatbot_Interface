#include <QApplication>
#include <QMainWindow>
#include <QSplitter>
#include <QListWidget>
#include <QVBoxLayout>
#include "ChatWindow.h"


int main(int argc, char* argv[]){
    QApplication app(argc, argv);
    QMainWindow window;
    window.setWindowTitle("Chat");
    QSplitter* splitter = new QSplitter(Qt::Horizontal);

    QListWidget* leftPanel = new QListWidget();
    leftPanel->setStyleSheet(R"(
            QListWidget {
                background-color: palette(base);
                padding: 5px;
                outline: none;
                color: palette(text);
            }

            QListWidget::item {
                background-color: transparent;
                padding: 12px;
                border-radius: 6px;
                color: palette(text);
                border: 1px solid transparent;
            }

            QListWidget::item:selected {
                background-color: palette(highlight);
                color: palette(highlighted-text);
            }

            QListWidget::item:selected:hover {
                background-color: palette(highlight);
            }
        )");


    ChatManager manager(leftPanel);
    ChatWindow* rightPanel = new ChatWindow(&manager);
    QObject::connect(leftPanel, &QListWidget::itemClicked, [rightPanel](QListWidgetItem* item) {
        if (item) {
            ChatMetadata chatData = item->data(Qt::UserRole).value<ChatMetadata>();
            rightPanel->clearAndDisplayChat(chatData);
        }
    });
    QObject::connect(leftPanel, &QListWidget::currentItemChanged,
                    [rightPanel](QListWidgetItem* current, QListWidgetItem* previous) {
        if (current) {
            ChatMetadata chatData = current->data(Qt::UserRole).value<ChatMetadata>();
            rightPanel->clearAndDisplayChat(chatData);
        }
    });



    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setSizes({384,1536});
    leftPanel->setMinimumWidth(100);
    rightPanel->setMinimumWidth(200);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(splitter);
    layout->setContentsMargins(0, 0, 0, 0);
    window.setCentralWidget(splitter);


    window.resize(1920,1080);
    window.show();

    return app.exec();
}
