#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("打印刻录安全监控系统");
    window.resize(1024, 768);

    QWidget *centralWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();

    QLabel *titleLabel = new QLabel("欢迎使用打印刻录安全监控系统");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");

    layout->addWidget(titleLabel);
    centralWidget->setLayout(layout);
    window.setCentralWidget(centralWidget);

    window.show();
    return app.exec();
}