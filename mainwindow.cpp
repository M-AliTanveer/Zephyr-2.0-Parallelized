#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStack>
#include "threadproofer.h"
#include "exprtk.hpp"
#include <omp.h>
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
    int i, j;
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
   /*for(int i=0; i<partfuncs.size();i++)
   {
       ui->Chp4formulalabel->setText(ui->Chp4formulalabel->text() + "\n" + partfuncs[i]);
   }*/
   Xpoints[0]=ui->Ch4x0input->text().toFloat();
   Xpoints[1]=ui->Ch4x1input->text().toFloat();
   Xpoints[2]=ui->Ch4x2input->text().toFloat();
   Xpoints[3]=ui->Ch4x3input->text().toFloat();
   Xpoints[4]=ui->Ch4x4input->text().toFloat();
   Xpoints[5]=ui->Ch4x5input->text().toFloat();
   Xpoints[6]=ui->Ch4x6input->text().toFloat();

   Ypoints[0]=ui->Ch4y0input->text().toFloat();
   Ypoints[1]=ui->Ch4y1input->text().toFloat();
   Ypoints[2]=ui->Ch4y2input->text().toFloat();
   Ypoints[3]=ui->Ch4y3input->text().toFloat();
   Ypoints[4]=ui->Ch4y4input->text().toFloat();
   Ypoints[5]=ui->Ch4y5input->text().toFloat();
   Ypoints[6]=ui->Ch4y6input->text().toFloat();
   forwarddiff();
}

void MainWindow::forwarddiff()
{
    double h = Xpoints[1] - Xpoints[0];
    int count = ui->Chp4pointsbox->currentText().toInt();
    double *answers = new double[count];
    ThreadProofer threads[8];
    double result;
    if(Ypoints[1]==Ypoints[0] && Ypoints[0]==0)
    {
         string input = ui->Chp4FuncInput->text().toStdString();
         string function = input.erase(input.find('='),input.length());
         double variable;
         exprtk::symbol_table<double> symbol_table;
         symbol_table.add_variable("x",variable);
         exprtk::expression<double> expression;
         expression.register_symbol_table(symbol_table);
         exprtk::parser<double> calculater;
         calculater.compile(function,expression);
         for (int i=0; i<count; i++)
         {
           variable = Xpoints[i];
           Ypoints[i] = expression.value();
         }
    }

#pragma omp parallel for num_threads(8)
    for(int i=0; i<count;i++)
    {
        if(i!=count-1)
        answers[i]=Ypoints[i+1]-Ypoints[i];
        if(i==count-1)
        answers[i]=Ypoints[i]-Ypoints[i-1];
        threads[i].threadid=omp_get_thread_num();
        answers[i]=answers[i]/h;
        threads[i].value=answers[i];

    }

    for(int i=0; i<count;i++)
    {
        ui->Chp4formulalabel->setText(ui->Chp4formulalabel->text() + "\n" + "key value pair: " + QString::number(threads[i].threadid) + " " + QString::number(threads[i].value));
    }

}

