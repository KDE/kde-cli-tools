// TODO GPL
#ifndef __keditfiletype_h
#define __keditfiletype_h

#include <kdialogbase.h>
#include <kmimetype.h>

class TypesListItem;
class FileTypeDetails;

// A dialog for ONE file type to be edited.
class FileTypeDialog : public KDialogBase
{
  Q_OBJECT
public:
  FileTypeDialog( KMimeType::Ptr mime );

protected slots:

  //virtual void slotDefault();
  //virtual void slotUser1(); // Reset
  virtual void slotApply();
  virtual void slotOk();
  void clientChanged(bool state);

private:
  FileTypeDetails * m_details;
  TypesListItem * m_item;
};

#endif

