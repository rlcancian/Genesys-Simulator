#ifndef EFSMNETWORKEDITORCONTROLLER_H
#define EFSMNETWORKEDITORCONTROLLER_H

class MainWindow;

/** GUI-only command handlers for EFSM network editing. */
class EFSMNetworkEditorController {
public:
    explicit EFSMNetworkEditorController(MainWindow* host);
    void createState();
    void createTransition();
    void editTransition();
    void setInitialState();
    void editState();
    bool hasNetworkContext() const;
    bool canCreateState() const;
    bool canCreateTransition() const;
    bool canEditTransition() const;
    bool canEditState() const;
    bool canSetInitialState() const;

private:
    MainWindow* _host = nullptr;
};

#endif // EFSMNETWORKEDITORCONTROLLER_H
