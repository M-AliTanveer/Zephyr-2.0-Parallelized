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

int MainWindow::gettopop(vector<QString> funcparts)
{
    int darr=funcparts.size();
    for(int i=0; i<darr;i++)
    {
        if(funcparts[i].toStdString()[0]=='/')
            return i;
    }
    for(int i=0; i<darr;i++)
    {
        if(funcparts[i].toStdString()[0]=='*')
            return i;
    }
    for(int i=0; i<darr;i++)
    {
        if(funcparts[i].toStdString()[0]=='+'||funcparts[i].toStdString()[0]=='-')
            return i;
    }

    return -1;
}

void MainWindow::funccalculator()
{
    vector<QString> partfuncs=funcbreaker();
    int count = ui->Chp4pointsbox->currentText().toInt();
    if(Ypoints[1]==Ypoints[0] && Ypoints[0]==0)
    {
         double variable;
         int darr;
         partfuncs.size()%2==0?(darr=partfuncs.size()/2):(darr=partfuncs.size()/2+1);
         vector<vector<double>> computed_vals;
#pragma omp parallel num_threads(darr)
    {
         exprtk::symbol_table<double> symbol_table;
         symbol_table.add_variable("x",variable);
         exprtk::expression<double> expression;
         expression.register_symbol_table(symbol_table);
         exprtk::parser<double> calculater;
#pragma omp for ordered
         for(int i=0; i<darr;i++)
         {
             vector<double> vals;
             string function = partfuncs[2*i].toStdString();
             calculater.compile(function,expression);
             for (int j=0; j<count; j++)
             {
               variable = Xpoints[j];
               vals.insert(vals.begin()+j,expression.value());
             }
#pragma omp ordered
             {computed_vals.insert(computed_vals.begin()+i,vals);}
         }
     }
         for(int i=0; i<darr;i++)
         {
             vector<double> ans;
             int op = gettopop(partfuncs);
             if(op!=-1)
            {
                 for(int j=0; j<count;j++)
                 {
                     if(partfuncs[op].toStdString()[0]=='/')
                         ans.insert(ans.begin()+j, computed_vals[op/2][j]/computed_vals[op/2+1][j]);
                     else if(partfuncs[op].toStdString()[0]=='*')
                         ans.insert(ans.begin()+j, computed_vals[op/2][j]*computed_vals[op/2+1][j]);
                     else if(partfuncs[op].toStdString()[0]=='+')
                         ans.insert(ans.begin()+j, computed_vals[op/2][j]+computed_vals[op/2+1][j]);
                     else if(partfuncs[op].toStdString()[0]=='-')
                         ans.insert(ans.begin()+j, computed_vals[op/2][j]-computed_vals[op/2+1][j]);

                 }
                 computed_vals.erase(computed_vals.begin()+(op/2));
                 computed_vals.insert(computed_vals.begin()+(op/2),ans);

                 computed_vals.erase(computed_vals.begin()+(op/2)+1);

                 partfuncs.erase(partfuncs.begin()+op-1,partfuncs.begin()+op+1);

             }

         }
         for(int i=0; i<count; i++)
         {
             Ypoints[i]=computed_vals[0][i];
         }
    }
}

void MainWindow::on_Ch4StartButton_clicked()
{
   ui->Chp4formulalabel->setText("");
   int count = ui->Chp4pointsbox->currentText().toInt();
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
   //forwarddiff();
   funccalculator();
   //forwarddiff();
   //3point();
   if(ui->Ch4choicebox->currentText() == "3 Point Mid & End"){
       threepoint();
   }
   else{
       forwarddiff();
   }
}

void MainWindow::forwarddiff()
{
    double h = Xpoints[1] - Xpoints[0];
    int count = ui->Chp4pointsbox->currentText().toInt();
    double *answers = new double[count];
    ThreadProofer threads[8];
    double result;
#pragma omp parallel for num_threads(count)
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
void MainWindow::threepoint()
{
  double h = Xpoints[1] - Xpoints[0];
  int count = ui->Chp4pointsbox->currentText().toInt();
  double *answers = new double[count];
  double q;
  ThreadProofer threads[8];
  if (count % 2 == 0)
  {
    int m2 = count / 2;
    int m1 = m2 - 1;
    #pragma omp parallel for num_threads(8)
    for (int i = 0; i < count; i++)
    {
      if ((i < m1) && (i < m2))
      {
        answers[i] = (-3*(Ypoints[i])+(4*(Ypoints[i+1]+1))-(Ypoints[i+2]));
        answers[i] = answers[i] / (2*h);
      }
      else if ((i == m1) || (i == m2))
      {
        answers[i] = (Ypoints[i+1] - Ypoints[i-1]);
        answers[i] = answers[i] / (2*h);
      }
      else if ((i > m1) && (i > m2))
      {
        answers[i] = (-3 * (Ypoints[i]) + 4 * (Ypoints[i-1]) - (Ypoints[i-2]));
        q = h - 2 * h;
        answers[i] = (answers[i]/(2*q));
      }
    }
  }
  else
  {
    int m = floor(count / 2);
    #pragma omp parallel for num_threads(8)
    for (int i = 0; i < count; i++)
    {
      if (i < m)
      {
        answers[i] = (-3*(Ypoints[i])+4*(Ypoints[i+1])-(Ypoints[i+2]));
        answers[i] = answers[i]/(2*h);
      }
      else if (i == m)
      {
        answers[i] = (Ypoints[i+1]-Ypoints[i-1]);
        answers[i] = answers[i] / (2 * h);
      }
      else if (i > m)
      {
        answers[i] = (-3 * (Ypoints[i]) + 4 * (Ypoints[i - 1]) - (Ypoints[i - 2]));
        q = h - 2 * h;
        answers[i] = answers[i] / (2 * q);
      }
    }
  }
  for(int i=0; i<count;i++)
  {
      ui->Chp4formulalabel->setText(ui->Chp4formulalabel->text() + "\n" + "key value pair: " + " " + QString::number(threads[i].value));
  }
}
void MainWindow::fivepoint(){
 double height = Xpoints[1] - Xpoints[0];
    int count = ui->Chp4pointsbox->currentText().toInt();
    double *answers = new double[count];
    ThreadProofer threads[8];
    int i=0;
    double DerivAns=0;
#pragma omp parallel for num_threads(count)
    for(int j =0; j<count; j++){
    while(i>count){
      if(i+5<=count){
        DerivAns+=-25*Ypoints[i];
            DerivAns+=48*Ypoints[i+1];
            DerivAns+=-36*Ypoints[i+2];
            DerivAns+=16*Ypoints[i+3];
            DerivAns+=-3*Ypoints[i+4];
            DerivAns=DerivAns/(12*height);
      }
      else if(i-4 >= 0){
            DerivAns+=-25*Ypoints[i];
            DerivAns+=48*Ypoints[i-1];
            DerivAns+=-36*Ypoints[i-2];
            DerivAns+=16*Ypoints[i-3];
            DerivAns+=-3*Ypoints[i-4];
            DerivAns=DerivAns/(-12*height);
      }
      else{
            DerivAns+=Ypoints[i-2];
            DerivAns+=-8*Ypoints[i-1];
            DerivAns+=8*Ypoints[i+1];
            DerivAns+=-1*Ypoints[i+2];
            DerivAns=DerivAns/(12*height);
      }
      i++;
    }
  }

}
