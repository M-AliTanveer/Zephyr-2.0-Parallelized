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

void MainWindow::funccalculator(int count,const double *xarr, double *yarr)
{
    vector<QString> partfuncs=funcbreaker();
    if(yarr[1]==yarr[0] && yarr[0]==0)
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
               variable = xarr[j];
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
             yarr[i]=computed_vals[0][i];
         }
    }
}

void MainWindow::on_Ch4StartButton_clicked()
{
   ui->Chp4formulalabel->setText("");
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
   if(ui->Ch4choicebox->currentText() == "3 Point Mid & End"){
       threepoint();
   }
   else if(ui->Ch4choicebox->currentText() == "5 Point Mid & End"){
       fivepoint();
   }
   else if(ui->Ch4choicebox->currentText() == "Forward Difference")
   {
       forwarddiff();
   }
   else if(ui->Ch4choicebox->currentText() == "Trapezoid")
   {
       compositetrapezoidal();
   }
}

void MainWindow::forwarddiff()
{
    int count = ui->Chp4pointsbox->currentText().toInt();
    funccalculator(count,Xpoints,Ypoints);
    double h = Xpoints[1] - Xpoints[0];
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
    int count = ui->Chp4pointsbox->currentText().toInt();
    funccalculator(count,Xpoints,Ypoints);
  double h = Xpoints[1] - Xpoints[0];

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
        answers[i] = (-3*(Ypoints[i])+(4*(Ypoints[i+1]))-(Ypoints[i+2]));
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
      threads[i].threadid=omp_get_thread_num();
      threads[i].value=answers[i];
    }
  }
  else
  {
    int m = floor(count / 2);
    #pragma omp parallel for num_threads(count)
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
      threads[i].threadid=omp_get_thread_num();
      threads[i].value=answers[i];
    }
  }
  for(int i=0; i<count;i++)
  {
      ui->Chp4formulalabel->setText(ui->Chp4formulalabel->text() + "\n" + "key value pair: " + QString::number(threads[i].threadid) + " " + QString::number(threads[i].value));
  }
}

void MainWindow::fivepoint()
{
    int count = ui->Chp4pointsbox->currentText().toInt();
    funccalculator(count,Xpoints,Ypoints);
    double height = Xpoints[1] - Xpoints[0];
    double *answers = new double[count];
    ThreadProofer threads[8];
    int i=0;
    double DerivAns=0;
#pragma omp parallel for num_threads(count)
    for(int j =0; j<count; j++){
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
      answers[i]=round(DerivAns);
      threads[i].value=answers[i];
  }
    for(int i=0; i<count;i++)
    {
        ui->Chp4formulalabel->setText(ui->Chp4formulalabel->text() + "\n" + "key value pair: " + " " + QString::number(threads[i].value));
    }
}

void MainWindow::compositetrapezoidal()
{
    double a = Xpoints[0];
    double b = Xpoints[1];

    double h = Ypoints[0];
    int count = (b-a)/h;
    count++;
    double *xvals = new double[count];
    xvals[0]=a;
    double *yvals = new double[count];
#pragma omp parallel for num_threads(count)
    for(int i=1; i<count;i++)
    {
        xvals[i]=xvals[0]+h*i;
    }
    for(int i=0;i<count;i++)
    {
        yvals[i]=0;
    }

    funccalculator(count,xvals,yvals);
    double ans=0;
#pragma omp parallel for reduction(+:ans) num_threads(count)
    for(int i =1; i<count-1;i++)
    {
        ans+=2*yvals[i];
    }
    ans=ans+yvals[0]+yvals[count-1];
    ans=ans*(h/2);

    ui->Chp4formulalabel->setText(ui->Chp4formulalabel->text() + "\n" + QString::number(ans));


}
