#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QSplitter>
#include <QListWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include "ChatWindow.h"
#include "ChatBubble.h"

int main(int argc, char* argv[]){
    QApplication app(argc, argv);
    QMainWindow window;
    window.setWindowTitle("Chat");
    QWidget* central = new QWidget;
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
    leftPanel->addItem("Chat 1");
    leftPanel->addItem("Chat 2");

    ChatWindow* rightPanel = new ChatWindow();

    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setSizes({384,1536});
    leftPanel->setMinimumWidth(100);
    rightPanel->setMinimumWidth(200);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(splitter);
    layout->setContentsMargins(0, 0, 0, 0);
    central->setLayout(layout);
    window.setCentralWidget(central);


    window.resize(3840,2160);
    window.show();

    return app.exec();
}
