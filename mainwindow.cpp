#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QString>
#include <QFileDialog>
#include <QPlainTextEdit>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init_Connections();
    ui->stackedWidget->setCurrentIndex(0);
    //ajouterFichierMenu();
}

MainWindow::~MainWindow()
{
    delete ui;
}
/**
 * @brief Initialisation des connexions pour les actions de l'interface.
 * @return void
 * @param void
 */
void MainWindow::init_Connections(){
    connect(ui->actionAjouter_fichier_txt, &QAction::triggered, this, &MainWindow::ajouterFichierMenu);
    connect(ui->actionCredit_de_fichier_txt, &QAction::triggered, this, &MainWindow::creditFichierMenu);
    connect(ui->tabWidgetFichier, &QTabWidget::tabCloseRequested, this, &MainWindow::close_onglet);
    connect(ui->actionEditer_les_fichiers_ouverts, &QAction::triggered, this, &MainWindow::editerFichierMenu);
    connect(ui->actionSauvegarder, &QAction::triggered, this, &MainWindow::sauvegarderFichierActuel);
}

void MainWindow::ajouterFichierMenu(){
    qDebug()<<"lecture fichier menu";
    ui->stackedWidget->setCurrentIndex(0);
    QString nom_fichier = QFileDialog::getOpenFileName(this, tr("Ouvrir un fichier"), "", tr("Fichiers texte (*.txt);;Tous les fichiers (*)"));

    if (!nom_fichier.isEmpty()){
        QFile *fichier = new QFile(nom_fichier);
        if (fichier->open(QIODevice::ReadOnly)){
            QTextStream in(fichier);
            QString contenu_fichier = in.readAll();
            this->liste_fichier_ouvert.append(fichier);
            fichier->close();

            QPlainTextEdit *editor = new QPlainTextEdit;
            //ICI J'ECRIS UNE FONDCTION LAMBDA QUI CAPTURE LES VARIABLES this, editor POUR VERIFIER QUE LE FICHIER EST MODIFIER OU NON ET ECRIRE * DANS LE STACK WIDGET
            connect(editor, &QPlainTextEdit::textChanged, this, [this, editor](){
                int index = ui->tabWidgetFichier->indexOf(editor);
                if(ui->tabWidgetFichier->tabText(index).endsWith("*")==false){
                    ui->tabWidgetFichier->setTabText(index, ui->tabWidgetFichier->tabText(index) + "*");
                }
            });
            editor->setPlainText(contenu_fichier);
            ui->tabWidgetFichier->addTab(editor, QFileInfo(nom_fichier).fileName());
            ui->tabWidgetFichier->setTabsClosable(true);
        }
        else{
            qDebug()<<"Erreur d'ouverture du fichier";
        }
    }
}

void MainWindow::sauvegarderFichierActuel(){
    sauvegarde_fichier(ui->tabWidgetFichier->currentIndex());
}
void MainWindow::creditFichierMenu(){
    qDebug()<<"Crédit fichier menu";
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::editerFichierMenu(){
    qDebug()<<"Editer les fichiers ouverts menu";
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::sauvegarde_fichier(int index) {
    if (index >= 0 && index < liste_fichier_ouvert.size()) {
        QFile *fichier = liste_fichier_ouvert.at(index);
        QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(ui->tabWidgetFichier->widget(index));
        if (editor && fichier->open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(fichier);
            out << editor->toPlainText();
            fichier->close();

            // Supprimer le marqueur de modification (*)
            QString tabName = ui->tabWidgetFichier->tabText(index);
            if(tabName.endsWith("*")) {
                tabName.chop(1);
                ui->tabWidgetFichier->setTabText(index, tabName);
            }
            QMessageBox::information(this, tr("Succés"), tr("Sauvegarde du fichier reussi avec succés."));
        } else {
            QMessageBox::warning(this, tr("Erreur"), tr("Impossible de sauvegarder le fichier."));
        }
    }
}


void MainWindow::close_onglet(int index){
    QWidget *widget = ui->tabWidgetFichier->widget(index);
    // Vérifier si l'onglet est marqué comme modifié
    if(ui->tabWidgetFichier->tabText(index).endsWith("*")) {
        QMessageBox::StandardButton reponse;
        reponse = QMessageBox::question(this, "Fichier non sauvegardé","Le fichier a été modifié. Voulez-vous sauvegarder les modifications?",QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

        if(reponse == QMessageBox::Yes) {
            this->sauvegarde_fichier(index);
            // Appeler une fonction pour sauvegarder le fichier, si nécessaire.
        } else if(reponse == QMessageBox::Cancel) {
            return;
        }
    }
    ui->tabWidgetFichier->removeTab(index);
    delete liste_fichier_ouvert.takeAt(index);  // Deletes the QFile from heap and removes it from the list
    delete widget;  // Libération de la mémoire
}
