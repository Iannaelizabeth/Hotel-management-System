#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_ROOMS 100
#define MAX_CUSTOMERS 100

// ================= STRUCTS =================

typedef struct {
    int roomNumber;
    float pricePerDay;
    int isOccupied;
    char occupantName[50];
    int checkInDate;
    int checkOutDate;
    int occupants;
} Room;

typedef struct {
    char name[50];
    int roomNumber;
    int checkInDate;
    int checkOutDate;
    int occupants;
    float totalBill;
} Customer;

typedef struct {
    char username[30];
    char password[30];
} Owner;

// ================= GLOBALS =================

Room rooms[MAX_ROOMS];
Customer customers[MAX_CUSTOMERS];
Owner owners[] = {
    {"admin", "admin123"},
    {"manager", "hotel2025"}
};
int totalRooms = 0;
int totalCustomers = 0;
int totalOwners = 2;

HWND hMainWnd;



// ================= CORE FUNCTIONS =================

Room* findRoom(int roomNumber) {
    for (int i = 0; i < totalRooms; i++) {
        if (rooms[i].roomNumber == roomNumber)
            return &rooms[i];
    }
    return NULL;
}

int ownerLoginCheck(const char* username, const char* password) {
    for (int i = 0; i < totalOwners; i++) {
        if (strcmp(owners[i].username, username) == 0 &&
            strcmp(owners[i].password, password) == 0) {
            return 1;
        }
    }
    return 0;
}

int addRoomInternal(int roomNumber, float pricePerDay) {
    if (totalRooms >= MAX_ROOMS)
        return 0; // failure

    if (findRoom(roomNumber) != NULL)
        return 0; // duplicate

    Room r;
    r.roomNumber = roomNumber;
    r.pricePerDay = pricePerDay;
    r.isOccupied = 0;
    rooms[totalRooms++] = r;
    return 1; // success
}

int bookRoomInternal(const char* customerName, int roomNumber, int occupants, int checkInDate, int checkOutDate, float* outBill) {
    Room* r = findRoom(roomNumber);
    if (r == NULL || r->isOccupied)
        return 0; // room not available

    Customer c;
    strcpy(c.name, customerName);
    c.roomNumber = roomNumber;
    c.occupants = occupants;
    c.checkInDate = checkInDate;
    c.checkOutDate = checkOutDate;
    c.totalBill = (checkOutDate - checkInDate) * r->pricePerDay;

    *outBill = c.totalBill;

    // update room info
    r->isOccupied = 1;
    strcpy(r->occupantName, customerName);
    r->occupants = occupants;
    r->checkInDate = checkInDate;
    r->checkOutDate = checkOutDate;

    customers[totalCustomers++] = c;
    return 1;
}

float getFinalBill(const char* customerName) {
    for (int i = 0; i < totalCustomers; i++) {
        if (strcmp(customers[i].name, customerName) == 0) {
            return customers[i].totalBill;
        }
    }
    return -1; // not found
}

int changePricingInternal(int roomNumber, float newPrice) {
    Room* r = findRoom(roomNumber);
    if (r) {
        r->pricePerDay = newPrice;
        return 1;
    }
    return 0;
}

int removeRoomInternal(int roomNumber) {
    for (int i = 0; i < totalRooms; i++) {
        if (rooms[i].roomNumber == roomNumber) {
            // Shift all rooms after this one
            for (int j = i; j < totalRooms - 1; j++) {
                rooms[j] = rooms[j + 1];
            }
            totalRooms--;
            return 1;
        }
    }
    return 0;
}


void getUpcomingCheckIns(int date, char* buffer, int bufferSize) {
    buffer[0] = '\0';
    for (int i = 0; i < totalCustomers; i++) {
        if (customers[i].checkInDate >= date) {
            char entry[100];
            sprintf(entry, "%s arriving on %d (Room %d)\n", 
                   customers[i].name, customers[i].checkInDate, customers[i].roomNumber);
            strcat(buffer, entry);
        }
    }
    if (buffer[0] == '\0') {
        strcat(buffer, "No upcoming check-ins\n");
    }
}

void getUpcomingCheckOuts(int date, char* buffer, int bufferSize) {
    buffer[0] = '\0';
    for (int i = 0; i < totalCustomers; i++) {
        if (customers[i].checkOutDate >= date) {
            char entry[100];
            sprintf(entry, "%s departing on %d (Room %d)\n", 
                   customers[i].name, customers[i].checkOutDate, customers[i].roomNumber);
            strcat(buffer, entry);
        }
    }
    if (buffer[0] == '\0') {
        strcat(buffer, "No upcoming check-outs\n");
    }
}

void getAvailableRooms(char* buffer, int bufferSize) {
    buffer[0] = '\0';
    for (int i = 0; i < totalRooms; i++) {
        if (!rooms[i].isOccupied) {
            char entry[100];
            sprintf(entry, "Room %d - $%.2f/day\n", rooms[i].roomNumber, rooms[i].pricePerDay);
            strcat(buffer, entry);
        }
    }
    if (buffer[0] == '\0') {
        strcat(buffer, "No available rooms.\n");
    }
}

void getReservationCalendar(char* buffer, int bufferSize) {
    buffer[0] = '\0';
    for (int i = 0; i < totalCustomers; i++) {
        char entry[100];
        sprintf(entry, "%s -> Room %d: %d to %d\n", customers[i].name, customers[i].roomNumber,
               customers[i].checkInDate, customers[i].checkOutDate);
        strcat(buffer, entry);
    }
    if (buffer[0] == '\0') {
        strcat(buffer,"No reservations.\n");
    }
}

void getGuestList(char* buffer, int bufferSize) {
    buffer[0] = '\0';
    for (int i = 0; i < totalCustomers; i++) {
        char entry[100];
        sprintf(entry, "%s in Room %d\n", customers[i].name, customers[i].roomNumber);
        strcat(buffer, entry);
    }
    if (buffer[0] == '\0') {
        strcat(buffer, "No guests currently.\n");
    }
}
// Forward declarations of dialogs






INT_PTR CALLBACK CustomerMenuDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK OwnerLoginDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK OwnerMenuDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AddRoomDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK BookRoomDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK FinalBillDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ChangePricingDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK RemoveRoomDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ListGuestsDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CheckInsDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CheckOutsDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK RoomServiceDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ReservationCalendarDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AvailableRoomsDlgProc(HWND, UINT, WPARAM, LPARAM);
// ================== GUI Utility Functions ==================

void ShowError(HWND hParent, const char* msg) {
    MessageBox(hParent, msg, "Error", MB_ICONERROR);
}

void ShowInfo(HWND hParent, const char* msg) {
    MessageBox(hParent, msg, "Information", MB_OK);
}

// ================== Dialogs Implementation ==================

// Customer Menu Dialog
INT_PTR CALLBACK CustomerMenuDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDCANCEL:
                    EndDialog(hwndDlg, 0);
                    return TRUE;

                case 101: // Book Room
                    DialogBox(NULL, MAKEINTRESOURCE(300), hwndDlg, BookRoomDlgProc);
                    return TRUE;

                case 102: // Final Bill
                    DialogBox(NULL, MAKEINTRESOURCE(301), hwndDlg, FinalBillDlgProc);
                    return TRUE;

                case 103: // Request Room Service
                    DialogBox(NULL, MAKEINTRESOURCE(302), hwndDlg, RoomServiceDlgProc);
                    return TRUE;

                case 104: // View Available Rooms
                    DialogBox(NULL, MAKEINTRESOURCE(303), hwndDlg, AvailableRoomsDlgProc);
                    return TRUE;

                case 105: // View Reservation Calendar
                    DialogBox(NULL, MAKEINTRESOURCE(304), hwndDlg, ReservationCalendarDlgProc);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}

// Owner Login Dialog
INT_PTR CALLBACK OwnerLoginDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static char username[30];
    static char password[30];
    switch(uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                GetDlgItemText(hwndDlg, 1001, username, 30);
                GetDlgItemText(hwndDlg, 1002, password, 30);

                if (ownerLoginCheck(username, password)) {
                    ShowInfo(hwndDlg, "Login successful");
                    EndDialog(hwndDlg, 1); // success
                } else {
                    ShowError(hwndDlg, "Invalid credentials");
                }
                return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, 0);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// Owner Menu Dialog
INT_PTR CALLBACK OwnerMenuDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDCANCEL:
                    EndDialog(hwndDlg, 0);
                    return TRUE;

                case 201: // Add Room
                    DialogBox(NULL, MAKEINTRESOURCE(305), hwndDlg, AddRoomDlgProc);
                    return TRUE;

                case 202: // Change Pricing
                    DialogBox(NULL, MAKEINTRESOURCE(306), hwndDlg, ChangePricingDlgProc);
                    return TRUE;

                case 203: // Remove Room
                    DialogBox(NULL, MAKEINTRESOURCE(307), hwndDlg, RemoveRoomDlgProc);
                    return TRUE;

                case 204: // List Guests
                    DialogBox(NULL, MAKEINTRESOURCE(308), hwndDlg, ListGuestsDlgProc);
                    return TRUE;

                case 205: // Check Upcoming Check-Ins
                    DialogBox(NULL, MAKEINTRESOURCE(309), hwndDlg, CheckInsDlgProc);
                    return TRUE;

                case 206: // Check Upcoming Check-Outs
                    DialogBox(NULL, MAKEINTRESOURCE(310), hwndDlg, CheckOutsDlgProc);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}

// Add Room Dialog
INT_PTR CALLBACK AddRoomDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char roomNumStr[20];
    char priceStr[20];
    int roomNum;
    float price;
    char msg[100];

    switch(uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                GetDlgItemText(hwndDlg, 1001, roomNumStr, 20);
                GetDlgItemText(hwndDlg, 1002, priceStr, 20);

                roomNum = atoi(roomNumStr);
                price = (float) atof(priceStr);

                if (roomNum <= 0 || price <= 0.0f) {
                    ShowError(hwndDlg, "Enter valid positive numbers.");
                    return TRUE;
                }

                if (addRoomInternal(roomNum, price)) {
                    sprintf(msg, "Room %d added at price $%.2f", roomNum, price);
                    ShowInfo(hwndDlg, msg);
                    EndDialog(hwndDlg, IDOK);
                } else {
                    ShowError(hwndDlg, "Room exists or max rooms reached.");
                }
                return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// Book Room Dialog
INT_PTR CALLBACK BookRoomDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char name[50], roomNumStr[20], occupantsStr[10], checkInStr[10], checkOutStr[10];
    int roomNum, occupants, checkIn, checkOut;
    float bill;
    char msg[200];

    switch(uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                GetDlgItemText(hwndDlg, 1001, name, 50);
                GetDlgItemText(hwndDlg, 1002, roomNumStr, 20);
                GetDlgItemText(hwndDlg, 1003, occupantsStr, 10);
                GetDlgItemText(hwndDlg, 1004, checkInStr, 10);
                GetDlgItemText(hwndDlg, 1005, checkOutStr, 10);

                roomNum = atoi(roomNumStr);
                occupants = atoi(occupantsStr);
                checkIn = atoi(checkInStr);
                checkOut = atoi(checkOutStr);

                if (strlen(name) == 0 || roomNum <= 0 || occupants <= 0 || checkIn <= 0 || checkOut <= checkIn) {
                    ShowError(hwndDlg, "Enter valid data. Check-out must be after check-in.");
                    return TRUE;
                }

                if (bookRoomInternal(name, roomNum, occupants, checkIn, checkOut, &bill)) {
                    sprintf(msg, "Room booked for %s.\nTotal bill: $%.2f", name, bill);
                    ShowInfo(hwndDlg, msg);
                    EndDialog(hwndDlg, IDOK);
                } else {
                    ShowError(hwndDlg, "Room unavailable or does not exist.");
                }
                return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// Final Bill Dialog
INT_PTR CALLBACK FinalBillDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char name[50], msg[100];
    float bill;

    switch(uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                GetDlgItemText(hwndDlg, 1001, name, 50);
                bill = getFinalBill(name);

                if (bill < 0) {
                    ShowError(hwndDlg, "Customer not found.");
                } else {
                    sprintf(msg, "Final bill for %s is $%.2f", name, bill);
                    ShowInfo(hwndDlg, msg);
                    EndDialog(hwndDlg, IDOK);
                }
                return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// Change Pricing Dialog
INT_PTR CALLBACK ChangePricingDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char roomNumStr[20], priceStr[20];
    int roomNum;
    float price;
    char msg[100];

    switch(uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                GetDlgItemText(hwndDlg, 1001, roomNumStr, 20);
                GetDlgItemText(hwndDlg, 1002, priceStr, 20);

                roomNum = atoi(roomNumStr);
                price = (float) atof(priceStr);

                if (roomNum <= 0 || price <= 0.0f) {
                    ShowError(hwndDlg, "Enter valid positive numbers.");
                    return TRUE;
                }

                if (changePricingInternal(roomNum, price)) {
                    sprintf(msg, "Price for Room %d updated to $%.2f", roomNum, price);
                    ShowInfo(hwndDlg, msg);
                    EndDialog(hwndDlg, IDOK);
                } else {
                    ShowError(hwndDlg, "Room not found.");
                }
                return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// Remove Room Dialog
INT_PTR CALLBACK RemoveRoomDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char roomNumStr[20];
    int roomNum;
    char msg[100];

    switch(uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                GetDlgItemText(hwndDlg, 1001, roomNumStr, 20);
                roomNum = atoi(roomNumStr);

                if (roomNum <= 0) {
                    ShowError(hwndDlg, "Enter valid room number.");
                    return TRUE;
                }

                if (removeRoomInternal(roomNum)) {
                    sprintf(msg, "Room %d removed successfully.", roomNum);
                    ShowInfo(hwndDlg, msg);
                    EndDialog(hwndDlg, IDOK);
                } else {
                    ShowError(hwndDlg, "Room not found.");
                }
                return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// List Guests Dialog
INT_PTR CALLBACK ListGuestsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char buffer[2000];
    
    switch(uMsg) {
        case WM_INITDIALOG:
            getGuestList(buffer, sizeof(buffer));
            SetDlgItemText(hwndDlg, 1001, buffer);
            return TRUE;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, LOWORD(wParam));
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// Add these new dialog procedures
INT_PTR CALLBACK CheckInsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char dateStr[20], buffer[2000];
    int date;
    
    switch(uMsg) {
        case WM_INITDIALOG:
            SetWindowText(hwndDlg, "Upcoming Check-Ins");
            return TRUE;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                GetDlgItemText(hwndDlg, 1001, dateStr, 20);
                date = atoi(dateStr);
                
                getUpcomingCheckIns(date, buffer, sizeof(buffer));
                SetDlgItemText(hwndDlg, 1002, buffer);
                return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

INT_PTR CALLBACK CheckOutsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char dateStr[20], buffer[2000];
    int date;
    
    switch(uMsg) {
        case WM_INITDIALOG:
            SetWindowText(hwndDlg, "Upcoming Check-Outs");
            return TRUE;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                GetDlgItemText(hwndDlg, 1001, dateStr, 20);
                date = atoi(dateStr);
                
                getUpcomingCheckOuts(date, buffer, sizeof(buffer));
                SetDlgItemText(hwndDlg, 1002, buffer);
                return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// Room Service Dialog
INT_PTR CALLBACK RoomServiceDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char roomNumStr[20];
    int roomNum;
    
    switch(uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                GetDlgItemText(hwndDlg, 1001, roomNumStr, 20);
                roomNum = atoi(roomNumStr);
                
                if (roomNum <= 0) {
                    ShowError(hwndDlg, "Enter valid room number.");
                    return TRUE;
                }
                
                Room* r = findRoom(roomNum);
                if (r && r->isOccupied) {
                    ShowInfo(hwndDlg, "Room service requested. We'll be there shortly!");
                    EndDialog(hwndDlg, IDOK);
                } else {
                    ShowError(hwndDlg, "Room not found or not occupied.");
                }
                return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// Available Rooms Dialog
INT_PTR CALLBACK AvailableRoomsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char buffer[2000];
    
    switch(uMsg) {
        case WM_INITDIALOG:
            getAvailableRooms(buffer, sizeof(buffer));
            SetDlgItemText(hwndDlg, 1001, buffer);
            return TRUE;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, LOWORD(wParam));
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// Reservation Calendar Dialog
INT_PTR CALLBACK ReservationCalendarDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    char buffer[2000];
    
    switch(uMsg) {
        case WM_INITDIALOG:
            getReservationCalendar(buffer, sizeof(buffer));
            SetDlgItemText(hwndDlg, 1001, buffer);
            return TRUE;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, LOWORD(wParam));
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// ================== Main Window Procedure ==================

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            CreateWindow("BUTTON", "Owner Login", WS_VISIBLE | WS_CHILD, 20, 20, 120, 30, hwnd, (HMENU)1, NULL, NULL);
            CreateWindow("BUTTON", "Customer Menu", WS_VISIBLE | WS_CHILD, 20, 60, 120, 30, hwnd, (HMENU)2, NULL, NULL);
            CreateWindow("BUTTON", "Exit", WS_VISIBLE | WS_CHILD, 20, 100, 120, 30, hwnd, (HMENU)3, NULL, NULL);
            break;
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case 1: {
                    int ret = DialogBox(NULL, MAKEINTRESOURCE(100), hwnd, OwnerLoginDlgProc);
                    if (ret == 1) { // Logged in
                        DialogBox(NULL, MAKEINTRESOURCE(101), hwnd, OwnerMenuDlgProc);
                    }
                    break;
                }
                case 2:
                    DialogBox(NULL, MAKEINTRESOURCE(102), hwnd, CustomerMenuDlgProc);
                    break;
                case 3:
                    PostQuitMessage(0);
                    break;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ================== WinMain ==================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "HotelMainClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Failed to register window class.", "Error", MB_ICONERROR);
        return 0;
    }

    hMainWnd = CreateWindow("HotelMainClass", "Hotel Management System", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 200, NULL, NULL, hInstance, NULL);

    if (!hMainWnd) {
        MessageBox(NULL, "Window creation failed.", "Error", MB_ICONERROR);
        return 0;
    }

    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int) msg.wParam;
}
