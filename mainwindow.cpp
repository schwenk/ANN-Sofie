#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer(this);

    ErrorView = new QGraphicsScene;

    //Initalize completly connected stacked neural net
    Cluster0 = new NeuralCluster(numInputs,numOutputs,numHiddens);
    ClusterBP= new NeuralCluster(numInputs,numOutputs,numHiddens);

    image = new QImage(Cluster0->getActivation().size()*2,Cluster0->getActivation().size()*2,QImage::Format_RGB32);
    imageResp = new QImage((numInputs+numOutputs+numHiddens)*4,numLessons*numOutputs*4*2,QImage::Format_RGB32);

    lastErrorBP = 0.0;
    lastErrorMine = 0.0;
    currentErrorBP = 0.0;
    CurrentErrorMine = 0.0;

}

MainWindow::~MainWindow()
{
    delete ui;
}

vector<float> MainWindow::inputFunction(int type, int length,int periode,int phase){
    vector<float> function;

    if(type == 0){
        int toggle = 0;
        for(int i = 0; i < length; i++){
            if(((i+phase)%(periode+1)) == 0) toggle = 1-toggle;
            if(toggle == 0) function.push_back(1.0);
            if(toggle == 1) function.push_back(0.0);
        }
    }
    if(type == 1){
        for(int i = 0; i < length; i++){
            if(((i+phase)%(periode+1)) == 0) function.push_back(1.0);
            else function.push_back(0.0);
        }
    }
    if(type == 2){
        for(int i = 0; i < length; i++){
          function.push_back(0.5-0.5*sin(8.0*3.16*(i+phase+(length/2))/(length*periode)));
        }
    }
    return function;
}

void MainWindow::processNet(){
        float sumErrorOver = 0.0;
        float sumErrorOverBP = 0.0;

        //Test pass error calculation
        currentFrequency = (currentFrequency+1)%(numOutputs-numLessons-1);
        int frequency = currentFrequency+1;//(((currentFrequency)%(numOutputs+8)));
        if(currentFrequency == 1) phase = (phase+1)%numOutputs;

        phase = 0;
        //frequency = 4;
        vector<vector<float>> impulseResonses;
        for(int o = 0; o < numOutputs-numLessons; o++){
            frequency = o+1;
            for(int k = 0; k < numLessons; k++){



                //Create empty vector as output placeholder
                    vector<float> emptyV;
                //Create input vector for holding the input data (Frequency is random Waveform depends on the lesson number (is mapped to output-neurons))
                    vector<float> inputV = MainWindow::inputFunction(k,numInputs,frequency,phase);

                    vector<float> targetV;
                    for(int j = 0; j < numOutputs; j++){
                        targetV.push_back(0.0);
                        //cout << inputV[j];
                    }
                    targetV[k] = 1.0;
                    targetV[numLessons+frequency-1] = 1.0;


                    for(int i = 0; i < 16; i++){
                        Cluster0->propergate(inputV,emptyV,false);
                        ClusterBP->propergate(inputV,emptyV,false);
                    }
                    vector<float> out0 = Cluster0->getActivation();
                    vector<float> out1 = ClusterBP->getActivation();

                    for(int i = 0; i < numOutputs; i++){
                        sumErrorOver += (targetV[i]-out0[i+numInputs])*(targetV[i]-out0[i+numInputs]);
                        sumErrorOverBP += (targetV[i]-out1[i+numInputs])*(targetV[i]-out1[i+numInputs]);
                        //out0[i+numInputs] = abs(targetV[i]-out0[i+numInputs]);
                    }

                    impulseResonses.push_back(out0);
                    impulseResonses.push_back(out1);
           }

           for(int k = 0; k < numLessons; k++){
            //Training pass


               //Create empty vector as output placeholder
                   vector<float> emptyV;
               //Create input vector for holding the input data (Frequency is random Waveform depends on the lesson number (is mapped to output-neurons))
                   vector<float> inputV = MainWindow::inputFunction(k,numInputs,frequency,phase);

                   vector<float> targetV;
                   for(int j = 0; j < numOutputs; j++){
                       targetV.push_back(0.0);
                       //cout << inputV[j];
                   }
                   targetV[k] = 1.0;
                   targetV[numLessons+frequency-1] = 1.0;



                    for(int i = 0; i < 16; i++){
                        Cluster0->propergate(inputV,targetV,false);
                        Cluster0->train(1.0);
                    }

                    vector<float> mergedTarget;

                    for(int i = 0; i < numInputs; i++){
                        mergedTarget.push_back(inputV[i]);
                    }

                    for(int i = 0; i < numOutputs; i++){
                        mergedTarget.push_back(targetV[i]);
                    }


                    for(int i = 0; i < 16; i++){
                        ClusterBP->propergate(inputV,emptyV,true);
                        ClusterBP->trainBP(mergedTarget,1.0,5);
                    }



            }
      }


        for(int x = 0; x < impulseResonses.size()*4; x++){
            for(int y = 0; y < impulseResonses[0].size()*4; y++){
                QColor col = QColor(128,128,128);
                float colorVal = 2.0*(impulseResonses[x/4][y/4]-0.5);
                if(colorVal > 0.0) col = QColor(255.0*abs(colorVal),0,0);
                if(colorVal <= 0.0) col = QColor(0,0,255.0*abs(colorVal));
                imageResp->setPixel(y,x,col.rgb());
            }
        }

        float max = -999999.9;
        for(int x = 0; x < Cluster0->getActivation().size()*2; x++){
            for(int y = 0; y < Cluster0->getActivation().size()*2; y++){
                QColor col = QColor(128,128,128);
                if(abs(Cluster0->getWeights()[y/2][x/2]) > max) max = abs(Cluster0->getWeights()[y/2][x/2]);
            }
        }

        for(int x = 0; x < Cluster0->getActivation().size()*2; x++){
            for(int y = 0; y < Cluster0->getActivation().size()*2; y++){
                QColor col = QColor(128,128,128);

                if(Cluster0->getWeights()[y/2][x/2] > 0.0) col = QColor(255.0*Cluster0->getWeights()[y/2][x/2]/max,0,0);
                if(Cluster0->getWeights()[y/2][x/2] < 0.0) col = QColor(0,0,-255.0*Cluster0->getWeights()[y/2][x/2]/max);

                image->setPixel(x,y,col.rgb());
            }
        }




        //image = new QImage(ClusterBP->getActivation().size()*2,ClusterBP->getActivation().size()*2,QImage::Format_RGB32);

/*
        for(int x = 0; x < ClusterBP->getActivation().size()*2; x++){
            for(int y = 0; y < ClusterBP->getActivation().size()*2; y++){
                QColor col = QColor(128,128,128);

                if(ClusterBP->getWeights()[y/2][x/2] > 0.0) col = QColor(255.0*1.0/(1.0+exp(-ClusterBP->getWeights()[y/2][x/2])),0,0);
                if(ClusterBP->getWeights()[y/2][x/2] < 0.0) col = QColor(0,0,255.0*1.0/(1.0+exp(ClusterBP->getWeights()[y/2][x/2])));

                image->setPixel(x,y,col.rgb());
            }
        }
*/

        lastErrorMine = CurrentErrorMine;
        CurrentErrorMine = sumErrorOver;

        lastErrorBP = currentErrorBP;
        currentErrorBP = sumErrorOverBP;

        QPen coloredLine;
        QColor col = QColor(255,128,128);
        coloredLine.setColor(col);
        ErrorView->addLine(iteration,0,iteration+1,0);
        ErrorView->addLine(iteration,-lastErrorBP*4,iteration+1,-currentErrorBP*4);
        ErrorView->addLine(iteration,-lastErrorMine*4,iteration+1,-CurrentErrorMine*4,coloredLine);


           QGraphicsScene* scene = new QGraphicsScene;
           scene->addPixmap(QPixmap::fromImage(*image));

           QGraphicsScene* scene2 = new QGraphicsScene;
           scene2->addPixmap(QPixmap::fromImage(*imageResp));

           ui->graphicsView_2->setScene(scene2);
           ui->graphicsView->setScene( scene );

           ui->graphicsView_3->setScene(ErrorView);

           ui->graphicsView->show();
           ui->graphicsView_2->show();
           ui->graphicsView_3->show();

           iteration += 1;
}

void MainWindow::on_pushButton_clicked()
{
    if(running == false){
        connect(timer, SIGNAL(timeout()), this, SLOT(processNet()));
        timer->start();
        running = !running;
    }else {
        timer->stop();
        running = !running;
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    this->processNet();
}
