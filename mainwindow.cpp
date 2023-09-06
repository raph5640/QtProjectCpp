//Auteur : Raphael De Oliveira
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QString>
#include <QFileDialog>
#include <QPlainTextEdit>
#include <QMessageBox>
#include <QInputDialog>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    editor_settings = new QSettings("Organisation", "EditorC++", this);
    init_Connections();
    init_shortcut();
    init_Enables();
    ui->stackedWidget->setCurrentIndex(0);

    for (int i = 0; i < MaxRecentFiles; ++i) {
        QAction *action = new QAction(this);
        action->setVisible(false);
        connect(action, &QAction::triggered, this, &MainWindow::ouvrirFichierRecent);
        ui->menuFichiers_recent->addAction(action);
        recentFileActs.append(action);
    }
    updateFichierRecent();
}

MainWindow::~MainWindow()
{
    delete editor_settings;
    delete ui;
}
void MainWindow::init_Enables(bool b){
    ui->actionChercher_du_texte->setEnabled(b);
    ui->action_Remplacer->setEnabled(b);
    ui->actionSauvegarder->setEnabled(b);
    ui->actionEnregistrer_sous->setEnabled(b);
}

/*!
 * \brief Initialise les connexions pour les actions de l'interface.
 */
void MainWindow::init_Connections(){
    connect(ui->actionAjouter_fichier_txt, &QAction::triggered, this, &MainWindow::ouvrirFichierMenu);          //Connection ajouter un fichier
    connect(ui->actionCredit_de_fichier_txt, &QAction::triggered, this, &MainWindow::creditFichierMenu);        //Connection crédit menu action
    connect(ui->tabWidgetFichier, &QTabWidget::tabCloseRequested, this, &MainWindow::close_onglet);             //Connection la croix de fermeture "x" avec le slot close_onglet
    connect(ui->actionEditer_les_fichiers_ouverts, &QAction::triggered, this, &MainWindow::editerFichierMenu);  //Connection permettant de revenir a l'espace de travail
    connect(ui->actionSauvegarder, &QAction::triggered, this, &MainWindow::sauvegarderFichierActuel);           //Connection pour la sauvegarde de fichier
    connect(ui->actionChercher_du_texte, &QAction::triggered, this, &MainWindow::chercherText);                 //Connection pour recherche du text dans l'editeur (Ctrl+F)
    connect(ui->action_Remplacer, &QAction::triggered, this,&MainWindow::remplacerText);
    connect(ui->menuMon_Espace, &QMenu::aboutToShow, this, &MainWindow::editerFichierMenu);
    //J'initialise toutes les actions correspondant a chacun des fichiers recents afin de pouvoir les ouvrir via le menu
    for (QAction *action : recentFileActs) {
        connect(action, &QAction::triggered, this, &MainWindow::ouvrirFichierRecent);
    }
    connect(ui->actionOpen_des_fichiers, &QAction::triggered, this, &MainWindow::ouvrirToutFichierRecent);
    connect(ui->actionNew_File, &QAction::triggered, this, &MainWindow::newfile);
    connect(ui->actionEnregistrer_sous, &QAction::triggered, this, &MainWindow::enregistrer_sous);
}
/*!
 * \brief Initialise les raccourcis pour les actions de l'interface.
 */
void MainWindow::init_shortcut(){
    ui->actionSauvegarder->setShortcut(QKeySequence("Ctrl+S"));
    ui->actionAjouter_fichier_txt->setShortcut(QKeySequence("Ctrl+A"));
    ui->actionChercher_du_texte->setShortcut(QKeySequence("Ctrl+F"));
    ui->action_Remplacer->setShortcut(QKeySequence("Ctrl+R"));
}
/*!
 * \brief Ajoute un nouveau fichier à l'espace de travail en ouvrant le répertoire.
 */
void MainWindow::ouvrirFichierMenu(){
    qDebug()<<"lecture fichier menu";
    ui->stackedWidget->setCurrentIndex(0);
    QString nom_fichier = QFileDialog::getOpenFileName(this, tr("Ouvrir un fichier"), "", tr("Tous les fichiers (*)"));
    for (int i = 0; i < liste_fichier_ouvert.size(); ++i) {
        if (liste_fichier_ouvert[i]->fileName() == nom_fichier) {
            ui->tabWidgetFichier->setCurrentIndex(i);
            QMessageBox::warning(this, tr("Erreur"), tr("Le fichier %1 est déja ouvert !").arg(nom_fichier));
            return;
        }
    }
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
            connect(editor, &QPlainTextEdit::cursorPositionChanged,this,&MainWindow::updateCursor);
            editor->setPlainText(contenu_fichier);
            ui->tabWidgetFichier->addTab(editor, QFileInfo(nom_fichier).fileName());
            ui->tabWidgetFichier->setTabsClosable(true);
            //Activer les actions "Chercher" et "Remplacer"
            init_Enables(true);
        }
        else{
            qDebug()<<"Erreur d'ouverture du fichier";
        }
        //Ajouter le fichier à la liste des derniers fichiers ouverts (dans mon attribut privé editor_settings)
        QStringList Fichier_recent = editor_settings->value("Fichier recent").toStringList();
        Fichier_recent.removeAll(nom_fichier);
        Fichier_recent.prepend(nom_fichier);
        while (Fichier_recent.size() > 10)
            Fichier_recent.removeLast();
        editor_settings->setValue("Fichier recent", Fichier_recent);
        updateFichierRecent();
    }
}

/*!
 * \brief Sauvegarde le fichier actuellement édité.
 */
void MainWindow::sauvegarderFichierActuel(){
    sauvegarde_fichier(ui->tabWidgetFichier->currentIndex());
}

void MainWindow::enregistrer_sous(){
    sauvegarder_sous(ui->tabWidgetFichier->currentIndex());
}
/*!
 * \brief Affiche les crédits de l'application.
 */
void MainWindow::creditFichierMenu(){
    qDebug()<<"Crédit fichier menu";
    ui->stackedWidget->setCurrentIndex(1);
}
/*!
 * \brief Passe en mode édition pour les fichiers ouverts.
 */
void MainWindow::editerFichierMenu(){
    qDebug()<<"Editer les fichiers ouverts menu";
    ui->stackedWidget->setCurrentIndex(0);
}

/*!
 * \brief Sauvegarde le fichier contenu à l'index donné.
 * \param index L'index du fichier à sauvegarder.
 */
void MainWindow::sauvegarde_fichier(int index) {
    qDebug()<<"Vous faites une sauvegarde du fichier en cours de modifications";
    if (index >= 0 && index < liste_fichier_ouvert.size()) {
        QFile *fichier = liste_fichier_ouvert.at(index);
        QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(ui->tabWidgetFichier->widget(index));
        if(fichier->exists()){
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
                QMessageBox::information(this, tr("Succés"), tr("Sauvegarde du fichier %1 reussi avec succés.").arg(tabName));
            } else {
                QMessageBox::warning(this, tr("Erreur"), tr("Impossible de sauvegarder le fichier."));
            }
        }else{
            this->sauvegarder_sous(index);
        }
    }
}
/*!
 * \brief Sauvegarde sous le fichier contenu à l'index donné.
 * \param index L'index du fichier à sauvegarder.
 */
void MainWindow::sauvegarder_sous(int index){
    QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(ui->tabWidgetFichier->widget(index));
    QString repertoire;
    QFile *Fichier_courant = liste_fichier_ouvert.at(index);
    if(Fichier_courant && Fichier_courant->exists()) {
        repertoire = QFileInfo(*Fichier_courant).absolutePath();
    }

    QString newFileName = QFileDialog::getSaveFileName(this, tr("Sauvegarder le fichier sous"), repertoire, tr("Tous les fichiers (*)"));
    if(newFileName.isEmpty()) {
        return;
    }

    QFile newFile(newFileName);
    if(newFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&newFile);
        out << editor->toPlainText();
        newFile.close();

        //On Supprime le vieux fichier de la liste des fichiers ouvert et on ajoute le nouveau
        delete liste_fichier_ouvert.takeAt(index);
        liste_fichier_ouvert.insert(index, new QFile(newFileName));

        //Mise à jour de l'onglet pour afficher le nouveau nom de fichier
        ui->tabWidgetFichier->setTabText(index, QFileInfo(newFileName).fileName());
        QMessageBox::information(this, tr("Succés"), tr("Le fichier a été sauvegardé avec succès sous: %1").arg(newFileName));
    } else {
        QMessageBox::warning(this, tr("Erreur"), tr("Impossible de sauvegarder le fichier sous: %1").arg(newFileName));
    }

}
/*!
 * \brief Ferme l'onglet. Si le fichier est en cours de modification, propose de sauvegarder ou de revenir en arrière.
 * \param index L'index de l'onglet à fermer.
 */
void MainWindow::close_onglet(int index){
    QWidget *widget = ui->tabWidgetFichier->widget(index);
    if(ui->tabWidgetFichier->tabText(index).endsWith("*")) {
        QMessageBox::StandardButton reponse;
        reponse = QMessageBox::question(this, "Fichier non sauvegardé","Le fichier a été modifié. Voulez-vous sauvegarder les modifications?",QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

        if(reponse == QMessageBox::Yes) {
            this->sauvegarde_fichier(index);

        } else if(reponse == QMessageBox::Cancel) {
            return;
        }
    }

    ui->tabWidgetFichier->removeTab(index);
    if(ui->tabWidgetFichier->count()==0){
        init_Enables(false);
    }
    delete liste_fichier_ouvert.takeAt(index);
    delete widget;  // Libération de la mémoire
}
/*!
 * \brief Met à jour l'affichage de la position du curseur dans la barre de statut.
 */
void MainWindow::updateCursor(){
    QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(ui->tabWidgetFichier->currentWidget());
    if(editor){
        QTextCursor cursor = editor->textCursor();
        int ligne = cursor.blockNumber() +1;
        int colonne = cursor.columnNumber() +1;
        ui->statusbar->showMessage(tr("Ligne : %1     Colonne : %2").arg(ligne).arg(colonne));
    }
}
/*!
 * \brief Lance une recherche de texte dans l'éditeur.
 */
void MainWindow::chercherText(){
    qDebug()<<"Vous faites une recherche de text dans votre editeur";
    QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(ui->tabWidgetFichier->currentWidget());
    editor->moveCursor(QTextCursor::Start);
    if(!editor) {
        return;
    }
    qDebug() << editor->toPlainText();
    QString text = QInputDialog::getText(this, tr("Recherche"), tr("Entrez le texte à rechercher:"));
    if(text.isEmpty()) {
        return;
    }
    QTextCursor found = editor->document()->find(text, editor->textCursor(), QTextDocument::FindWholeWords);
    if(found.isNull()) {
        QMessageBox::warning(this, tr("Recherche"), tr("Votre texte %1 n'a pas été trouvé").arg(text));
    } else {
        editor->setTextCursor(found);
    }
}
/*!
 * \brief Remplace un texte donné par un autre dans l'éditeur.
 */
void MainWindow::remplacerText(){
    QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(ui->tabWidgetFichier->currentWidget());
    editor->moveCursor(QTextCursor::Start);
    if(!editor) {
        return;
    }
    //Les boites de dialogues
    QString rechercheText = QInputDialog::getText(this, tr("Recherche"), tr("Texte à chercher "));
    QString replaceText = QInputDialog::getText(this, tr("Remplacement"), tr("Remplacer %1 par ").arg(rechercheText));

    //On vérifie que l'on a bien rentré du texte dans les deux boîtes de dialogues
    if(rechercheText.isEmpty() || replaceText.isEmpty()) {
        return;
    }
    //Effectuer une recherche tout en respectant la casse
    QTextDocument::FindFlags drapeau;
    drapeau = drapeau | QTextDocument::FindCaseSensitively;

    QTextCursor currentCursor = editor->textCursor();

    //Cherche la première occurrence
    QTextCursor found = editor->document()->find(rechercheText, currentCursor, drapeau);
    while(!found.isNull()) {
        found.insertText(replaceText);
        found = editor->document()->find(rechercheText, found, drapeau);
    }
}


void MainWindow::updateFichierRecent() {
    QStringList recentFiles = editor_settings->value("Fichier recent").toStringList();
    int numRecentFiles = qMin(recentFiles.size(), MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(recentFiles[i]).fileName());
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(recentFiles[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j){
        recentFileActs[j]->setVisible(false);
    }
}

void MainWindow::ouvrirFichierRecent() {
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        QString fileName = action->data().toString();
        ajouterFichierMenuText(fileName);
    }
}
void MainWindow::ajouterFichierMenuText(const QString &fileName) {
    if (fileName.isEmpty()) return;

    //Vérifie si le fichier est déjà ouvert
    for (int i = 0; i < liste_fichier_ouvert.size(); ++i) {
        if (liste_fichier_ouvert[i]->fileName() == fileName) {
            ui->tabWidgetFichier->setCurrentIndex(i);
            return;
        }
    }

    QFile *fichier = new QFile(fileName);
    if (fichier->open(QIODevice::ReadOnly)) {
        QTextStream in(fichier);
        QString contenu_fichier = in.readAll();
        fichier->close();

        this->liste_fichier_ouvert.append(fichier);

        QPlainTextEdit *editor = new QPlainTextEdit;
        connect(editor, &QPlainTextEdit::textChanged, this, [this, editor]() {
            int index = ui->tabWidgetFichier->indexOf(editor);
            if (!ui->tabWidgetFichier->tabText(index).endsWith("*")) {
                ui->tabWidgetFichier->setTabText(index, ui->tabWidgetFichier->tabText(index) + "*");
            }
        });

        connect(editor, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateCursor);
        editor->setPlainText(contenu_fichier);
        ui->tabWidgetFichier->addTab(editor, QFileInfo(fileName).fileName());
        ui->tabWidgetFichier->setTabsClosable(true);

        // Activer les actions "Chercher" et "Remplacer"
        init_Enables(true);
    } else {
        qDebug() << "Erreur d'ouverture du fichier";
    }
}

void MainWindow::ouvrirToutFichierRecent() {
    qDebug()<<"Vous ouvrez les 10 derniers fichiers recemments ouvert";
    QStringList fichiersRécents = editor_settings->value("Fichier recent").toStringList();
    for (const QString &fichier : fichiersRécents) {
        ajouterFichierMenuText(fichier);
    }
}

/*!
 * \brief Crée un nouveau fichier vierge dans un onglet.
 */
void MainWindow::newfile(){
    qDebug()<<"Création d'un nouvel onglet pour un nouveau fichier";

    QPlainTextEdit *editor = new QPlainTextEdit;
    connect(editor, &QPlainTextEdit::textChanged, this, [this, editor](){
        int index = ui->tabWidgetFichier->indexOf(editor);
        if(ui->tabWidgetFichier->tabText(index).endsWith("*")==false){
            ui->tabWidgetFichier->setTabText(index, ui->tabWidgetFichier->tabText(index) + "*");
        }
    });
    connect(editor, &QPlainTextEdit::cursorPositionChanged,this,&MainWindow::updateCursor);

    ui->tabWidgetFichier->addTab(editor, tr("NEW :)"));
    ui->tabWidgetFichier->setTabsClosable(true);

    // Activer les actions "Chercher" et "Remplacer"
    init_Enables(true);

    // Ajouter un fichier vide à la liste des fichiers ouverts
    QFile *fichier = new QFile();
    liste_fichier_ouvert.append(fichier);
}
