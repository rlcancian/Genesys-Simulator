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

private:
    MainWindow* _host = nullptr;
};

#endif // EFSMNETWORKEDITORCONTROLLER_H
