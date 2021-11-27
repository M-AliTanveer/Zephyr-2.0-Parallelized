#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStack>
using namespace std;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
vector<QString> MainWindow::funcbreaker()
{
    QStack<QChar>bracket;
    vector<QString> partfuncs;
    QString input = ui->Chp4FuncInput->text();
    //ui->Chp4formulalabel->setText(input);
    QChar *arr = new QChar[input.length()];
    int i, j,k;
    bool flag=false;
    for(i=0,j=0;i<input.length();i++,j++)
    {
        if(input[i]=='(')
        {
            flag=true;
            bracket.push('(');
        }
        if(input[i]==')')
        {
            bracket.pop();
            if(bracket.empty())
            flag=false;
        }
        if(input[i]==' ')
        {
            j--;
            continue;
        }
        if(input[i]=='='&&flag==false)
        {
            arr[j] = '\0';
            QString temp(arr);
            partfuncs.push_back(temp);
            j=-1;
            break;
        }
        arr[j]=input[i];
        if((input[i]=='+'||input[i]=='*'||input[i]=='-'||input[i]=='/')&&flag==false)
        {
            arr[j] = '\0';
            QString temp(arr);
            partfuncs.push_back(temp);
            partfuncs.push_back(input[i]);

            j=-1;
        }

    }

    return partfuncs;
}


void MainWindow::on_Ch4StartButton_clicked()
{
   vector<QString> partfuncs=funcbreaker();
   ui->Chp4formulalabel->setText("");
   for(int i=0; i<partfuncs.size();i++)
   {
       ui->Chp4formulalabel->setText(ui->Chp4formulalabel->text() + "\n" + partfuncs[i]);
   }
}

