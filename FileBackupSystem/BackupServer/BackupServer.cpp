#include <iostream>
#include <exception>
#include "../BackupServerCore/Server.h"

// הגדרת פורט ברירת מחדל (אלא אם כן תרצה שנוסיף קריאה מקובץ)
const int DEFAULT_PORT = 1234;

int main()
{
    try {
        std::cout << "Initializing Backup Server..." << std::endl;

        // יצירת מופע של השרת
        Server server(DEFAULT_PORT);

        // הרצת הלולאה הראשית (זו פונקציה חוסמת שלא חוזרת)
        server.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal Server Error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Fatal Server Error: Unknown exception occurred." << std::endl;
        return 1;
    }

    return 0;
}