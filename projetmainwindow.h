#ifndef PROJETMAINWINDOW_H
#define PROJETMAINWINDOW_H


#include <QWidget>
#include <QMainWindow>
#include "projet.h"
#include <QSqlQuery>
#include <QVariant>
#include <QChartView>
#include <QListWidget>
#include "employe.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ProjetMainWindow;
}
QT_END_NAMESPACE

class ProjetMainWindow : public QWidget
{
    Q_OBJECT

public:
   // ProjetMainWindow(QWidget *parent = nullptr);
    explicit ProjetMainWindow(const employe& e, QWidget *parent);
    ~ProjetMainWindow();
private slots:

    //
    void updateChart();
    //


    void on_affichageproj_activated(const QModelIndex &index);



    void on_pushButton_57_clicked();

    void on_pushButton_56_clicked();

    //void on_pushButton_58_clicked();//

    //
    void on_pushButton_60_clicked();

    void resetFormFields();
    void setFormTitle(const QString &title);
    //

    void on_comboBox_4_activated(int index);

    void on_recherche_4_textChanged(const QString &arg1);

    void on_pushButton_51_clicked();

    void on_cal_clicked(const QDate &date);



    void on_npr_currentTextChanged(const QString &arg1);

    void on_pushButton_59_clicked();

    void on_pushButton_54_clicked();
    //refresh cal
    void on_pushButton_55_clicked();




    //to do list

    void updateTaskStatusInDatabase(int taskId, const QString &newStatus);

    void setupDragDrop();
    void handleItemMoved(QListWidget *list);
    QString getStatusForList(QListWidget *list) const;
    void deleteSelectedTask();


    void addNewTask();
    //add to do list
    void on_pushButton_52_clicked();

    //sup todo
    void on_supp_task_clicked();



    //modif tache

    void setupListConnections() ;
    void handleItemEdit(QListWidgetItem *item) ;
    //Supp
    void clearAllSelectionsExcept(QListWidget* activeList);
    //projetspecifique
    void loadProjectsToComboBox();

    void loadTasksFromDatabase(int idProjet = -1);
    void refreshProjectsList();

    void on_Projet_todo_currentTextChanged(const QString &arg1);
    int insertTaskToDatabase(const QString &name, const QString &status, int idProjet);
    //suppression de tache des proj anulle
    void deleteTasksForProject(int projectId);
    //rapport
      QString genererResume(int id_projet);
    void on_pushButton_58_clicked();

      //arduino
    void update_label();
private:
    Ui::ProjetMainWindow *ui;

    Projet p;
    QChart *chart;
    QChartView *chartView;
    bool firstLoad = true;

    //arduino
    QByteArray data; // variable contenant les données reçues
    QString id;
    Arduino *arduino4;
};



#endif // PROJETMAINWINDOW_H
