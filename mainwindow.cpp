#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QString>
#include <QFileDialog>
#include <QPlainTextEdit>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init_Connections();
    lectureFichierMenu();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init_Connections(){
    connect(ui->actionLecture_fichier_txt, &QAction::triggered, this, &MainWindow::lectureFichierMenu);
    connect(ui->actionEcriture_de_fichier_txt, &QAction::triggered, this, &MainWindow::ecritureFichierMenu);
}

void MainWindow::lectureFichierMenu(){
    qDebug()<<"lecture fichier menu";
    ui->stackedWidget->setCurrentIndex(0);
    QString fileName = QFileDialog::getOpenFileName(this, tr("Ouvrir un fichier"), "", tr("Fichiers texte (*.txt);;Tous les fichiers (*)"));

    if (!fileName.isEmpty()){
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)){
            QTextStream in(&file);
            QString fileContent = in.readAll();
            file.close();

            QPlainTextEdit *editor = new QPlainTextEdit;
            editor->setPlainText(fileContent);

            ui->tabWidget->addTab(editor, QFileInfo(fileName).fileName());
            ui->tabWidget->setTabsClosable(true);

            connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
        }
        else{
            qDebug()<<"Erreur d'ouverture du fichier";
        }
    }
}

void MainWindow::ecritureFichierMenu(){
    qDebug()<<"ecriture fichier menu";
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::closeTab(int index){
    QWidget *widget = ui->tabWidget->widget(index);
    ui->tabWidget->removeTab(index);
    delete widget;  // Libération de la mémoire
}
