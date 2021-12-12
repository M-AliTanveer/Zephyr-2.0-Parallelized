#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
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
    void threepoint();
    void fivepoint();
    void funccalculator(int count, const double *xarr, double *yarr);
    int gettopop(std::vector<QString> funcparts);
    void compositetrapezoidal();
    void simpsonthird();
    void simpsoneight();
    void compmidpoint();


private slots:
    void on_Ch4StartButton_clicked();
    void on_Ch4choicebox_currentTextChanged(const QString &arg1);

    void on_Chp4pointsbox_currentIndexChanged(int index);

    void on_Ch4iterback_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
