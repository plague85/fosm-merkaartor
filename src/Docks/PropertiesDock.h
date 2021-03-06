#ifndef MERKATOR_PROPERTIESDOCK_H_
#define MERKATOR_PROPERTIESDOCK_H_

#include <ui_MinimumTrackPointProperties.h>
#include <ui_MinimumRelationProperties.h>
#include <ui_MinimumRoadProperties.h>
#include <ui_MultiProperties.h>

#include <QList>

#include "MDockAncestor.h"
#include "ShortcutOverrideFilter.h"

class MainWindow;
class Feature;
class TagModel;
class EditCompleterDelegate;
class TagTemplates;
class TagTemplate;
class CommandList;

class PropertiesDock : public MDockAncestor
{
    Q_OBJECT

    public:
        PropertiesDock(MainWindow* aParent);
    public:
        ~PropertiesDock(void);

        void setSelection(Feature* aFeature);
        void setMultiSelection(Feature* aFeature);
        template<class T>
                void setSelection(const QList<T*>& aFeatureList);
        void setMultiSelection(const QList<Feature*>& aFeatureList);
        void toggleSelection(Feature* aFeature);
        void addSelection(Feature* aFeature);
        Feature* selection(int idx);
        QList<Feature*> selection();
        bool isSelected(Feature *aFeature);
        int size() const;
        void resetValues();
        void checkMenuStatus();
        bool loadTemplates(const QString& filename = "");
        bool mergeTemplates(const QString& filename = "");
        bool saveTemplates(const QString& filename);

        Feature* highlighted(int idx);
        QList<Feature*> highlighted();
        int highlightedSize() const;


    public slots:
        void on_TrackPointLat_editingFinished();
        void on_TrackPointLon_editingFinished();
        void on_Member_customContextMenuRequested(const QPoint & pos);
        void on_Member_clicked(const QModelIndex & index);
        void on_Member_selected();
        void on_btMemberUp_clicked();
        void on_btMemberDown_clicked();
        void on_RemoveMemberButton_clicked();
        void on_RemoveTagButton_clicked();
        void on_SourceTagButton_clicked();
        void on_SelectionList_itemSelectionChanged();
        void on_SelectionList_itemDoubleClicked(QListWidgetItem* item);
        void executePendingSelectionChange();
        void on_SelectionList_customContextMenuRequested(const QPoint & pos);
        void on_centerAction_triggered();
        void on_centerZoomAction_triggered();
        void on_tag_changed(QString k, QString v);
        void on_tag_cleared(QString k);
        void on_template_changed(TagTemplate* aNewTemplate);
        void adjustSelection();

    private:
        void cleanUpUi();
        void switchUi();
        void switchToNoUi();
        void switchToNodeUi();
        void switchToWayUi();
        void switchToMultiUi();
        void switchToRelationUi();
        void fillMultiUiSelectionBox();
        void changeEvent(QEvent*);
        void retranslateUi();

        MainWindow* Main;
        QWidget* CurrentUi;
        QList<Feature*> Selection;
        QList<Feature*> FullSelection;
        Ui::TrackPointProperties TrackPointUi;
        Ui::RoadProperties RoadUi;
        Ui::MultiProperties MultiUi;
        Ui::RelationProperties RelationUi;
        TagModel* theModel;
        int PendingSelectionChange;
        EditCompleterDelegate* delegate;
        QAction* centerAction;
        QAction* centerZoomAction;
        QAction* selectAction;
        ShortcutOverrideFilter* shortcutFilter;
        TagTemplates* theTemplates;

        QList<Feature*> Highlighted;

        QTableView *CurrentTagView;
        QTableView *CurrentMembersView;

        enum { NoUiShowing, TrackPointUiShowing, RoadUiShowing, RelationUiShowing, MultiShowing } NowShowing ;
};

template<class T>
        void PropertiesDock::setSelection(const QList<T*>& aFeatureList)
{
    cleanUpUi();
    Selection.clear();
    for (int i=0; i<aFeatureList.size(); ++i)
        Selection.push_back(aFeatureList[i]);
    FullSelection = Selection;
    switchUi();
    fillMultiUiSelectionBox();
}


#endif


