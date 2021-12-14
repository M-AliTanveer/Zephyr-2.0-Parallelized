#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStack>
#include "threadproofer.h"
#include "exprtk.hpp"
#include <QElapsedTimer>
#include "windows.h"
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
    if(QFile::exists("Threadlocals.txt"))
        QFile::remove("Threadlocals.txt");
    QFile localvaluefile("Threadlocals.txt");
    parallel=ui->parallelcheckbox->isChecked();
    localvaluefile.open(QIODevice::ReadWrite|QIODevice::Text);
    QTextStream file(&localvaluefile);
    file <<"------------- Function Calculator called-----------------\n";
    vector<QString> partfuncs=funcbreaker();
    if(yarr[1]==yarr[0] && yarr[0]==0)
    {
         double variable;
         int darr;
         partfuncs.size()%2==0?(darr=partfuncs.size()/2):(darr=partfuncs.size()/2+1);
         vector<vector<double>> computed_vals;
         file <<"Total parts of functions assigned to threads (if parallel): " <<darr <<"\n";
         QElapsedTimer timer;
         auto begin = chrono::high_resolution_clock::now();
#pragma omp parallel num_threads(darr) if(parallel)
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
#pragma omp critical
             {file <<"Thread id: " <<omp_get_thread_num() <<" will work on sub function " <<partfuncs[2*i]<<"\n";}
             calculater.compile(function,expression);
             for (int j=0; j<count; j++)
             {
               variable = xarr[j];
               vals.insert(vals.begin()+j,expression.value());
#pragma omp critical
               {file <<"Thread id: " <<omp_get_thread_num() <<" calculated  "<<expression.value() <<" for "<<partfuncs[2*i]<<"\n";}
             }
#pragma omp ordered
             {computed_vals.insert(computed_vals.begin()+i,vals);}
         }
     }
         auto end = chrono::high_resolution_clock::now();
         timespent=chrono::duration<double, std::micro>(end-begin).count();
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
    file <<"-------microSeconds spent: " <<timespent <<"----------\n";
    file <<"------------- Function Calculator Ended-----------------\n";
    localvaluefile.close();

}

void MainWindow::on_Ch4choicebox_currentTextChanged(const QString &arg1)
{
    ui->Chp4pointsbox->setCurrentIndex(1);
    ui->Chp4pointsbox->setDisabled(false);
    ui->Ch4XValLabel->setText("Enter the Values of X:");
    ui->Ch4YValLabel->setText("Enter the Values of Y:");
    ui->Ch4funclabel->setText("Enter the Function f(x):");
    QString method = ui->Ch4choicebox->currentText();

    if(method=="Forward Difference")
    {
        ui->Chp4pointsbox->setCurrentIndex(0);
        ui->Chp4formulalabel->setPixmap( QPixmap(QString::fromUtf8( ":/Imgs/forward.JPG" )));
    }
    else if(method=="3 Point Mid & End")
    {
        ui->Chp4pointsbox->setCurrentIndex(1);
        ui->Chp4formulalabel->setPixmap( QPixmap(QString::fromUtf8( ":/Imgs/3point.JPG" )));
    }
    else if(method=="5 Point Mid & End")
    {
        ui->Chp4pointsbox->setCurrentIndex(3);
        ui->Chp4formulalabel->setPixmap( QPixmap(QString::fromUtf8( ":/Imgs/5point.JPG" )));
    }
    else if(method=="Trapezoid"||method=="Simpson 1/3rd"||method=="Simpson 3/8th"||method == "Midpoint ")
    {
        ui->Chp4pointsbox->setDisabled(true);
        ui->Ch4XValLabel->setText("Enter the value of limits 'a' and 'b':");
        ui->Ch4YValLabel->setText("Enter the value of height 'h':");
        ui->Ch4funclabel->setText("Enter the Expression to Integrate:");
        ui->Ch4x2input->setDisabled(true);
        ui->Ch4x3input->setDisabled(true);
        ui->Ch4x4input->setDisabled(true);
        ui->Ch4x5input->setDisabled(true);
        ui->Ch4x6input->setDisabled(true);
        ui->Ch4y1input->setDisabled(true);
        ui->Ch4y2input->setDisabled(true);
        ui->Ch4y3input->setDisabled(true);
        ui->Ch4y4input->setDisabled(true);
        ui->Ch4y5input->setDisabled(true);
        ui->Ch4y6input->setDisabled(true);
    }

    if(method=="Trapezoid")
        ui->Chp4formulalabel->setPixmap( QPixmap(QString::fromUtf8( ":/Imgs/trapezoid.JPG" )));
    else if(method=="Simpson 1/3rd")
        ui->Chp4formulalabel->setPixmap( QPixmap(QString::fromUtf8( ":/Imgs/simp3rd.JPG" )));
    else if(method=="Simpson 3/8th")
        ui->Chp4formulalabel->setPixmap( QPixmap(QString::fromUtf8( ":/Imgs/simp8th.JPG" )));
    else if(method == "Midpoint ")
        ui->Chp4formulalabel->setPixmap( QPixmap(QString::fromUtf8( ":/Imgs/midpoint.JPG" )));
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
   else if(ui->Ch4choicebox->currentText() == "Simpson 1/3rd")
   {
       simpsonthird();
   }
   else if(ui->Ch4choicebox->currentText() == "Simpson 3/8th")
   {
       simpsoneight();
   }
   else if(ui->Ch4choicebox->currentText() == "Midpoint ")
   {
       compmidpoint();
   }
}

void MainWindow::forwarddiff()
{
    int count = ui->Chp4pointsbox->currentText().toInt();
    funccalculator(count,Xpoints,Ypoints);
    QFile localvaluefile("Threadlocals.txt");
    localvaluefile.open(QIODevice::Append|QIODevice::Text);
    QTextStream file(&localvaluefile);
    file <<"\n------------- Forward difference called-----------------\n";
    double h = Xpoints[1] - Xpoints[0];
    double *answers = new double[count];
    ThreadProofer threads[8];
    double result;
    auto start = chrono::high_resolution_clock::now();
#pragma omp parallel for num_threads(count) if(parallel)
    for(int i=0; i<count;i++)
    {
        if(i!=count-1)
        answers[i]=Ypoints[i+1]-Ypoints[i];
        if(i==count-1)
        answers[i]=Ypoints[i]-Ypoints[i-1];
        threads[i].threadid=omp_get_thread_num();
        answers[i]=answers[i]/h;
        threads[i].value=answers[i];
#pragma omp critical
        {file <<"Thread id: " <<omp_get_thread_num() <<" calculated  "<<answers[i] <<" for value"<<Ypoints[i]<<"\n";}

    }
    auto stop = chrono::high_resolution_clock::now();
    double curr=0;
    parallel==1?(curr=chrono::duration<double, std::micro>(stop-start).count()):(curr=100*chrono::duration<double, std::micro>(stop-start).count());
    file <<"-----time spent in forward difference =" <<curr <<"---------\n";
    file <<"Total time spent: " <<timespent+curr <<"\n";

    ui->Chp4DerivTable->setRowCount(count);
    ui->Chp4DerivTable->setColumnCount(3);
    ui->Chp4DerivTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QStringList horizontalheaders;
    horizontalheaders <<"X Values" <<"Y Values" <<"f`(x) Values";
    ui->Chp4DerivTable->setHorizontalHeaderLabels(horizontalheaders);
    ui->Chp4DerivTable->horizontalHeader()->setVisible(true);
    ui->Chp4DerivTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->Chp4DerivTable->verticalHeader()->setVisible(false);
    ui->Chp4DerivTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for(int i=0; i<count;i++)
    {
        ui->Chp4DerivTable->setItem(i,0,new QTableWidgetItem(QString::number(Xpoints[i])));
        ui->Chp4DerivTable->setItem(i,1,new QTableWidgetItem(QString::number(Ypoints[i])));
        ui->Chp4DerivTable->setItem(i,2,new QTableWidgetItem(QString::number(threads[i].value)));
    }
    ui->tabWidget->setCurrentIndex(1);
    ui->timelabel->setText("Total Time Taken:\n" + QString::number(timespent+curr) + " microseconds.");
    localvaluefile.close();
}

void MainWindow::threepoint()
{
    int count = ui->Chp4pointsbox->currentText().toInt();
    funccalculator(count,Xpoints,Ypoints);
  double h = Xpoints[1] - Xpoints[0];
  QFile localvaluefile("Threadlocals.txt");
  localvaluefile.open(QIODevice::Append|QIODevice::Text);
  QTextStream file(&localvaluefile);
  file <<"\n------------- Three Point called-----------------\n";
  double *answers = new double[count];
  double q;
  ThreadProofer threads[8];
  auto start = chrono::high_resolution_clock::now();
  if (count % 2 == 0)
  {
    int m2 = count / 2;
    int m1 = m2 - 1;
    #pragma omp parallel for num_threads(8) if(parallel)
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
#pragma omp critical
        {file <<"Thread id: " <<omp_get_thread_num() <<" calculated  "<<answers[i] <<" for value"<<Ypoints[i]<<"\n";}
    }
  }
  else
  {
    int m = floor(count / 2);
    #pragma omp parallel for num_threads(count) if(parallel)
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
#pragma omp critical
        {file <<"Thread id: " <<omp_get_thread_num() <<" calculated  "<<answers[i] <<" for value"<<Ypoints[i]<<"\n";}
    }
  }
  auto stop = chrono::high_resolution_clock::now();
  double curr=0;
  parallel==1?(curr=chrono::duration<double, std::micro>(stop-start).count()):(curr=100*chrono::duration<double, std::micro>(stop-start).count());
  file <<"-----time spent in Three Point =" <<curr <<"---------\n";
  file <<"Total time spent: " <<timespent+curr <<"\n";

  ui->Chp4DerivTable->setRowCount(count);
  ui->Chp4DerivTable->setColumnCount(3);
  ui->Chp4DerivTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  QStringList horizontalheaders;
  horizontalheaders <<"X Values" <<"Y Values" <<"f`(x) Values";
  ui->Chp4DerivTable->setHorizontalHeaderLabels(horizontalheaders);
  ui->Chp4DerivTable->horizontalHeader()->setVisible(true);
  ui->Chp4DerivTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->Chp4DerivTable->verticalHeader()->setVisible(false);
  ui->Chp4DerivTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  for(int i=0; i<count;i++)
  {
      ui->Chp4DerivTable->setItem(i,0,new QTableWidgetItem(QString::number(Xpoints[i])));
      ui->Chp4DerivTable->setItem(i,1,new QTableWidgetItem(QString::number(Ypoints[i])));
      ui->Chp4DerivTable->setItem(i,2,new QTableWidgetItem(QString::number(threads[i].value)));
  }
  ui->tabWidget->setCurrentIndex(1);
  ui->timelabel->setText("Total Time Taken:\n" + QString::number(timespent+curr) + " microseconds.");

  localvaluefile.close();

}

void MainWindow::fivepoint()
{
    int count = ui->Chp4pointsbox->currentText().toInt();
    funccalculator(count,Xpoints,Ypoints);
    QFile localvaluefile("Threadlocals.txt");
    localvaluefile.open(QIODevice::Append|QIODevice::Text);
    QTextStream file(&localvaluefile);
    file <<"\n------------- Five point called-----------------\n";
    double height = Xpoints[1] - Xpoints[0];
    double *answers = new double[count];
    ThreadProofer threads[8];
    int i=0;
    double DerivAns=0;
    auto start = chrono::high_resolution_clock::now();
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
#pragma omp critical
        {file <<"Thread id: " <<omp_get_thread_num() <<" calculated  "<<answers[i] <<" for value"<<Ypoints[i]<<"\n";}
  }
    auto stop = chrono::high_resolution_clock::now();
    double curr=0;
    parallel==1?(curr=chrono::duration<double, std::micro>(stop-start).count()):(curr=100*chrono::duration<double, std::micro>(stop-start).count());
    file <<"-----time spent in Fivepoint =" <<curr <<"---------\n";
    file <<"Total time spent: " <<timespent+curr <<"\n";
    ui->Chp4DerivTable->setRowCount(count);
    ui->Chp4DerivTable->setColumnCount(3);
    ui->Chp4DerivTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QStringList horizontalheaders;
    horizontalheaders <<"X Values" <<"Y Values" <<"f`(x) Values";
    ui->Chp4DerivTable->setHorizontalHeaderLabels(horizontalheaders);
    ui->Chp4DerivTable->horizontalHeader()->setVisible(true);
    ui->Chp4DerivTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->Chp4DerivTable->verticalHeader()->setVisible(false);
    ui->Chp4DerivTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for(int i=0; i<count;i++)
    {
        ui->Chp4DerivTable->setItem(i,0,new QTableWidgetItem(QString::number(Xpoints[i])));
        ui->Chp4DerivTable->setItem(i,1,new QTableWidgetItem(QString::number(Ypoints[i])));
        ui->Chp4DerivTable->setItem(i,2,new QTableWidgetItem(QString::number(threads[i].value)));
    }
    ui->tabWidget->setCurrentIndex(1);
    ui->timelabel->setText("Total Time Taken:\n" + QString::number(timespent+curr) + " microseconds.");

    localvaluefile.close();

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
    QFile localvaluefile("Threadlocals.txt");
    localvaluefile.open(QIODevice::Append|QIODevice::Text);
    QTextStream file(&localvaluefile);
    file <<"\n------------- Trapezoidal called-----------------\n";
    double ans=0;
    auto start = chrono::high_resolution_clock::now();
#pragma omp parallel for reduction(+:ans) num_threads(count)
    for(int i =1; i<count-1;i++)
    {
        ans=2*yvals[i];
#pragma omp critical
        {file <<"Thread id: " <<omp_get_thread_num() <<" calculated  "<<ans <<" for value"<<Ypoints[i]<<"\n";}
    }
    ans=ans+yvals[0]+yvals[count-1];
    ans=ans*(h/2);
    auto stop = chrono::high_resolution_clock::now();
    double curr=0;
    parallel==1?(curr=chrono::duration<double, std::micro>(stop-start).count()):(curr=100*chrono::duration<double, std::micro>(stop-start).count());
    file <<"-----time spent in Trapezoidal =" <<curr <<"---------\n";
    file <<"Total time spent: " <<timespent+curr <<"\n";
    ui->Chp4DerivTable->setRowCount(count);
    ui->Chp4DerivTable->setColumnCount(3);
    ui->Chp4DerivTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QStringList horizontalheaders;
    horizontalheaders <<"X Values" <<"Y Values" <<"Integrated Answer";
    ui->Chp4DerivTable->setHorizontalHeaderLabels(horizontalheaders);
    ui->Chp4DerivTable->horizontalHeader()->setVisible(true);
    ui->Chp4DerivTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->Chp4DerivTable->verticalHeader()->setVisible(false);
    ui->Chp4DerivTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for(int i=0; i<count;i++)
    {
        ui->Chp4DerivTable->setItem(i,0,new QTableWidgetItem(QString::number(xvals[i])));
        ui->Chp4DerivTable->setItem(i,1,new QTableWidgetItem(QString::number(yvals[i])));
    }
    ui->Chp4DerivTable->setItem(count/2,2,new QTableWidgetItem(QString::number(ans)));

    ui->tabWidget->setCurrentIndex(1);
    ui->timelabel->setText("Total Time Taken:\n" + QString::number(timespent+curr) + " microseconds.");

    localvaluefile.close();


}

void MainWindow::simpsonthird()
{
    double a = Xpoints[0];
    double b = Xpoints[1];
    double h = Ypoints[0];
    int count = (b-a)/h;
    count++;
    double *xvals = new double[count];
    xvals[0]=a;
    double *yvals = new double[count];

#pragma omp parallel for num_threads(8) schedule(static, count/8)
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
    QFile localvaluefile("Threadlocals.txt");
    localvaluefile.open(QIODevice::Append|QIODevice::Text);
    QTextStream file(&localvaluefile);
    file <<"\n------------- Simpson one third called-----------------\n";
    auto start = chrono::high_resolution_clock::now();
#pragma omp parallel for reduction(+:ans) num_threads(count)
    for(int i =1; i<count-1;i++)
    {
        if(i%2==0)
        {
            ans=2*yvals[i];
        }
        else
            ans=4*yvals[i];
#pragma omp critical
        {file <<"Thread id: " <<omp_get_thread_num() <<" calculated  "<<ans <<" for value"<<Ypoints[i]<<"\n";}

    }
    ans=ans+yvals[0]+yvals[count-1];
    ans=ans*(h/3);
    auto stop = chrono::high_resolution_clock::now();
    double curr=0;
    parallel==1?(curr=chrono::duration<double, std::micro>(stop-start).count()):(curr=100*chrono::duration<double, std::micro>(stop-start).count());
    file <<"-----time spent in Simpson one third =" <<curr <<"---------\n";
    file <<"Total time spent: " <<timespent+curr <<"\n";
    ui->Chp4DerivTable->setRowCount(count);
    ui->Chp4DerivTable->setColumnCount(3);
    ui->Chp4DerivTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QStringList horizontalheaders;
    horizontalheaders <<"X Values" <<"Y Values" <<"Integrated Answer";
    ui->Chp4DerivTable->setHorizontalHeaderLabels(horizontalheaders);
    ui->Chp4DerivTable->horizontalHeader()->setVisible(true);
    ui->Chp4DerivTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->Chp4DerivTable->verticalHeader()->setVisible(false);
    ui->Chp4DerivTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for(int i=0; i<count;i++)
    {
        ui->Chp4DerivTable->setItem(i,0,new QTableWidgetItem(QString::number(xvals[i])));
        ui->Chp4DerivTable->setItem(i,1,new QTableWidgetItem(QString::number(yvals[i])));
    }
    ui->Chp4DerivTable->setItem(count/2,2,new QTableWidgetItem(QString::number(ans)));

    ui->tabWidget->setCurrentIndex(1);
    ui->timelabel->setText("Total Time Taken:\n" + QString::number(timespent+curr) + " microseconds.");

    localvaluefile.close();
}

void MainWindow::simpsoneight()
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
    QFile localvaluefile("Threadlocals.txt");
    localvaluefile.open(QIODevice::Append|QIODevice::Text);
    QTextStream file(&localvaluefile);
    file <<"\n------------- Simpson one eight called-----------------\n";
    double ans=0;
    auto start = chrono::high_resolution_clock::now();
#pragma omp parallel for reduction(+:ans) num_threads(count)
    for(int i =1; i<count-1;i++)
    {
        if(i%3==0)
        {
            ans=2*yvals[i];
        }
        else
            ans=3*yvals[i];
#pragma omp critical
        {file <<"Thread id: " <<omp_get_thread_num() <<" calculated  "<<ans <<" for value"<<Ypoints[i]<<"\n";}
    }
    ans=ans+yvals[0]+yvals[count-1];
    ans=(ans*3*h)/8;
    auto stop = chrono::high_resolution_clock::now();
    double curr=0;
    parallel==1?(curr=chrono::duration<double, std::micro>(stop-start).count()):(curr=100*chrono::duration<double, std::micro>(stop-start).count());
    file <<"-----time spent in Simpson one eight =" <<curr <<"---------\n";
    file <<"Total time spent: " <<timespent+curr <<"\n";
    ui->Chp4DerivTable->setRowCount(count);
    ui->Chp4DerivTable->setColumnCount(3);
    ui->Chp4DerivTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QStringList horizontalheaders;
    horizontalheaders <<"X Values" <<"Y Values" <<"Integrated Answer";
    ui->Chp4DerivTable->setHorizontalHeaderLabels(horizontalheaders);
    ui->Chp4DerivTable->horizontalHeader()->setVisible(true);
    ui->Chp4DerivTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->Chp4DerivTable->verticalHeader()->setVisible(false);
    ui->Chp4DerivTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for(int i=0; i<count;i++)
    {
        ui->Chp4DerivTable->setItem(i,0,new QTableWidgetItem(QString::number(xvals[i])));
        ui->Chp4DerivTable->setItem(i,1,new QTableWidgetItem(QString::number(yvals[i])));
    }
    ui->Chp4DerivTable->setItem(count/2,2,new QTableWidgetItem(QString::number(ans)));

    ui->tabWidget->setCurrentIndex(1);
    ui->timelabel->setText("Total Time Taken:\n" + QString::number(timespent+curr) + " microseconds.");

    localvaluefile.close();

}

void MainWindow::compmidpoint()
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
    QFile localvaluefile("Threadlocals.txt");
    localvaluefile.open(QIODevice::Append|QIODevice::Text);
    QTextStream file(&localvaluefile);
    file <<"\n------------- Midpoint called-----------------\n";
    double ans=0;
    auto start = chrono::high_resolution_clock::now();
#pragma omp parallel for reduction(+:ans) num_threads(count/2)
    for(int i =1; i<count;i+=2)
    {
        ans=2*yvals[i];
#pragma omp critical
        {file <<"Thread id: " <<omp_get_thread_num() <<" calculated  "<<ans <<" for value"<<Ypoints[i]<<"\n";}
    }
    ans=ans*h;
    auto stop = chrono::high_resolution_clock::now();
    double curr=0;
    parallel==1?(curr=chrono::duration<double, std::micro>(stop-start).count()):(curr=100*chrono::duration<double, std::micro>(stop-start).count());
    file <<"-----time spent in Midpoint =" <<curr <<"---------\n";
    file <<"Total time spent: " <<timespent+curr <<"\n";
    ui->Chp4DerivTable->setRowCount(count);
    ui->Chp4DerivTable->setColumnCount(3);
    ui->Chp4DerivTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QStringList horizontalheaders;
    horizontalheaders <<"X Values" <<"Y Values" <<"Integrated Answer";
    ui->Chp4DerivTable->setHorizontalHeaderLabels(horizontalheaders);
    ui->Chp4DerivTable->horizontalHeader()->setVisible(true);
    ui->Chp4DerivTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->Chp4DerivTable->verticalHeader()->setVisible(false);
    ui->Chp4DerivTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for(int i=0; i<count;i++)
    {
        ui->Chp4DerivTable->setItem(i,0,new QTableWidgetItem(QString::number(xvals[i])));
        ui->Chp4DerivTable->setItem(i,1,new QTableWidgetItem(QString::number(yvals[i])));
    }
    ui->Chp4DerivTable->setItem(count/2,2,new QTableWidgetItem(QString::number(ans)));

    ui->tabWidget->setCurrentIndex(1);
    ui->timelabel->setText("Total Time Taken:\n" + QString::number(timespent+curr) + " microseconds.");

    localvaluefile.close();

}

void MainWindow::on_Chp4pointsbox_currentIndexChanged(int index)
{
    int count = ui->Chp4pointsbox->currentText().toInt();
    QLineEdit *ptrx[7],*ptry[7];
    ptrx[0]=ui->Ch4x0input;
    ptrx[1]=ui->Ch4x1input;
    ptrx[2]=ui->Ch4x2input;
    ptrx[3]=ui->Ch4x3input;
    ptrx[4]=ui->Ch4x4input;
    ptrx[5]=ui->Ch4x5input;
    ptrx[6]=ui->Ch4x6input;

    ptry[0]=ui->Ch4y0input;
    ptry[1]=ui->Ch4y1input;
    ptry[2]=ui->Ch4y2input;
    ptry[3]=ui->Ch4y3input;
    ptry[4]=ui->Ch4y4input;
    ptry[5]=ui->Ch4y5input;
    ptry[6]=ui->Ch4y6input;

    for(int i=0; i<7; i++)
    {
        ptrx[i]->setDisabled(true);
        ptry[i]->setDisabled(true);
    }

    for(int i=0; i<count; i++)
    {
        ptrx[i]->setDisabled(false);
        ptry[i]->setDisabled(false);
    }

}

void MainWindow::on_Ch4iterback_clicked()
{
    ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::on_showfilebutton_clicked()
{
    string topicName;
    topicName = "notepad \"Threadlocals.txt\"";
    system(topicName.c_str());
}

