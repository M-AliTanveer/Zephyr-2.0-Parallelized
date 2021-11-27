#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    double Xpoints[7];
    double Ypoints[7];
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();;
    std::vector<QString> funcbreaker();
    void forwarddiff();


private slots:
    void on_Ch4StartButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
