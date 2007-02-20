/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


// QmitkFunctionality.h: interface for the QmitkFunctionality class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUSFUNCTIONALITY_H__1DC0BA6E_9B8D_4D63_8A63_5B661CE33712__INCLUDED_)
#define AFX_QUSFUNCTIONALITY_H__1DC0BA6E_9B8D_4D63_8A63_5B661CE33712__INCLUDED_

#include "QmitkBaseFunctionalityComponent.h"

#include <qobject.h>
#include <qstring.h>
#include <qtoolbutton.h>
#include <qaction.h>

class QActionGroup;

#include "mitkDataTree.h"
#include "mitkPropertyList.h"

class QmitkFctMediator;

/*!
\brief class providing a standard interface for MITK application functionality

A functionality contains the application main widget, a controls widget and the 
application logic. The functionality has to set up the main widget according to 
its needs when it is Activated, especially if it uses a shared main wiget. 
It does @em not need to reverse the settings when it is Deactivated, since it cannot
know if the subsequently Activated functionality will use that settings or some other
setting. Furthermore, most functionalities will connect interaction objects in Activated()
and disconnect them in Deactivated().

\section QmitkFunctionalityOptions Application option lists (Preferences)

Each functionality has a mitk::PropertyList, where it can store application options. By 
using this list you can recreate your application's status after the user closed and restarted
the application.

QmitkMainTemplate (and QmitkSampleApp) will read a list of options from file on startup and 
tell the functionalities about it. This base class of all functionalities will store away
your functionality's options in the m_Options variable. 

You should react on changes to this list in Activated(). OR, if you want to overload the 
SetFunctionalityOptionsList() method, make sure you know what you are doing and call
QmitkFunctionality::SetFunctionalityOptionsList().

\section QmitkFunctionalityOptionsDialog Application options dialog

QmitkMainTemplate could ask your functionality to provide an options dialog. In that case, CreateOptionWidget()
will be called. The default behaviour is to return NULL. Otherwise please create a QWidget with some option 
fields.
After the user closes the option dialog, the application will call OptionsChanged(QWidget*) on your functionality,
with the parameter being your widget from CreateOptionWidget().
OptionsChanged() is your chance to read the changed checkboxes, textedits, whatever from your QWidget*.

\warning There is no automatic way yet to create an option dialog from the m_Options list or automatically
         change the m_Options list from your widget. This should be solved some time in the future.
         A small collection of classes derived from mitk::PropertyObserver could be part of the solution. 
         They are able to display properties, react to changes in properties, and also change properties.

\section QmitkFunctionalityUserManual User manual

\warning This is work in progress. Not tested with DKFZ external functionalities.

 1. In your functionalities directory, create a subdirectory UserManual. (this is optional but useful)
 2. Inside UserManual, create a file that is called Qmitk<tt>YourFunctionalityName</tt>UserManual.dox
 3. Create a page Qmitk<tt>YourFunctionalityName</tt>UserManual inside that file:
\verbatim
\page QmitkEasySegmentationUserManual The Lymph Node Segmentation Functionality
\endverbatim
 4. If you now run doxygen and copy the output somewhere, set the option in SampleApp's option dialog ("Global options"),
    your documentation should be found when you press F1 inside your functionality.

\ingroup Functionalities
*/
class QmitkFunctionality : public QmitkBaseFunctionalityComponent
{
  Q_OBJECT
    bool m_Available;
  bool m_Activated;
public:

  /*!
  \brief default constructor for a QmitkFunctionality object
  @param parent the parent widget
  @param name the name of the functionality
  @param dataIt the name of the functionality widget
  */
  QmitkFunctionality(QObject *parent=0, const char *name=0, mitk::DataTreeIteratorBase* dataIt=NULL);

  /*!
  default destructor
  */
  virtual ~QmitkFunctionality();

  /*!
  \brief method for creating the widget containing the functionalities controls, like sliders, buttons etc.
  */
  virtual QWidget * CreateControlWidget(QWidget *parent) = 0;

  /*!
  \brief method for creating the widget that contains the functionality options
  */
  virtual QWidget * CreateOptionWidget(QWidget *parent);

  /*!
  \brief method for creating the functionality's main widget
  */
  virtual QWidget * CreateMainWidget(QWidget * parent) = 0;

  /*!
  \brief method for creating the connections of main and control widget
  */
  virtual void CreateConnections() {};

  /*!
  \brief method for creating a QAction object, i.e., toolbar button and menu entry
  */
  virtual QAction * CreateAction(QActionGroup * /*parent*/) { return NULL; };

  /*!
  \brief method for defining the name of the functionality
  */
  virtual QString GetFunctionalityName();

  /*!
  \brief called when a functionality becomes active/visible. Often, event-handlers are connected (e.g., 
  GlobalStateMachine::AddInteractor() or AddListener()) in Activated() and the connection is removed in Deactivated()
  (e.g., GlobalStateMachine::RemoveInteractor() or RemoveListener()).
  */
  virtual void Activated();

  /*!
  \brief called when a functionality is Deactivated, i.e., is no longer active/visible. Often, event-handlers are connected (e.g., 
  GlobalStateMachine::AddInteractor() or AddListener()) in Activated() and the connection is removed in Deactivated()
  (e.g., GlobalStateMachine::RemoveInteractor() or RemoveListener()).
  */
  virtual void Deactivated();


  //##Documentation
  //## @brief Is the functionality currently active?
  //## 
  //## @warning Will only work if in sub-classes the superclass-methods are called in Activated() and Deactivated()!
  virtual bool IsActivated();

  virtual bool IsAvailable();

  /*!
  \brief Returns \a true if method TreeChanged was called and
  is still active (did not return yet)
  */
  virtual bool IsInTreeChanged() const;

  /*!
  \brief setter method for data tree attribute
  @param it the reference to a data tree ieterator object
  */
  void SetDataTree(mitk::DataTreeIteratorBase* it);

  /*!
  \brief getter for dataTree attribute. It returns the 
  reference to a data tree iterator object
  */
  mitk::DataTreeIteratorBase* GetDataTreeIterator();

  virtual void TreeChanged(const itk::EventObject & treeChangedEvent);

  /*!
  \brief Overwrite this method to get informed of tree-changes
  when the functionality is active or activated

  \warning When a tree-change occurs while the method
  has already been called, it will not be called again to
  avoid the risk of infinite recursions.
  */
  virtual void TreeChanged();

  /*!
  \brief Gets called when the user changed something in the functionality's 
  option dialog. The functionaliy has to read the new values from the 
  option dialog widgets and apply them to the functionality
  */
  virtual void OptionsChanged(QWidget* optionDialog);

  /*!
  \brief Can return a list of properties that hold some state or options of the functionality.
  */
  virtual mitk::PropertyList* GetFunctionalityOptionsList();
  /*!
  \brief On startup of MITKSampleApp this will be called to restore options or state of the functionality.
  MITKSampleApp will create a PropertyList from some file and then call this method. Copy the information
  from this PropertyList! DO NOT SAVE THE POINTER, because the list will be deleted by MITKSampleApp after 
  this method returns!
  */
  virtual void SetFunctionalityOptionsList(mitk::PropertyList*);
  
  /**
   * Overrides the current application cursor with a wait cursor. 
   * Use WaitCursorOff() to restore it.
   */
  void WaitCursorOn();
  
  /**
   * Restores the applications cursor, which has been overridden using WaitCursorOn()
   */
  void WaitCursorOff();

signals:
  void Signal_dummy();
  void AvailabilityChanged(QmitkFunctionality*);
  void AvailabilityChanged();


protected:
  virtual void SetAvailability(bool available);
  /*!
  a reference to a data tree iterator object
  */
  mitk::DataTreeIteratorClone m_DataTreeIterator;

  bool m_TreeChangedWhileInActive;
  bool m_InTreeChanged;

  unsigned long m_ObserverTag;
  friend class QmitkFctMediator;

  mitk::PropertyList::Pointer m_Options;
};

#endif // !defined(AFX_QUSFUNCTIONALITY_H__1DC0BA6E_9B8D_4D63_8A63_5B661CE33712__INCLUDED_)
