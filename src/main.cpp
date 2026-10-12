// Entry point: parses options, loads the bank from the data folder, starts the HTTP server.
// Usage: bank_server [--port 8080] [--data data] [--frontend frontend]
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "ApiServer.h"
#include "Bank.h"
#include "BankService.h"

int main(int argc, char** argv) {
    std::string dataDir = "data";
    std::string frontendDir = "frontend";
    int port = 8080;

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        auto value = [&](std::string& target) {
            if (i + 1 >= argc) { std::cerr << "Missing value for " << a << "\n"; std::exit(2); }
            target = argv[++i];
        };
        if (a == "--port") {
            std::string p;
            value(p);
            try { port = std::stoi(p); } catch (...) { port = 0; }
            if (port < 1 || port > 65535) { std::cerr << "Invalid port\n"; return 2; }
        } else if (a == "--data") value(dataDir);
        else if (a == "--frontend") value(frontendDir);
        else { std::cerr << "Unknown option " << a << "\nUsage: bank_server [--port N] [--data DIR] [--frontend DIR]\n"; return 2; }
    }
    // Convenience: allow running from inside the build folder.
    namespace fs = std::filesystem;
    if (!fs::exists(frontendDir) && fs::exists("../frontend")) frontendDir = "../frontend";

    try {
        Bank bank(dataDir);
        BankService service(bank);
        ApiServer server(service, frontendDir);

        std::cout << "Bank Management System (educational simulation)\n"
                  << "  data folder    : " << fs::absolute(dataDir).string() << "\n"
                  << "  frontend folder: " << fs::absolute(frontendDir).string() << "\n"
                  << "  open in browser: http://localhost:" << port << "\n"
                  << "Press Ctrl+C to stop.\n";
        if (!server.listen("127.0.0.1", port)) {
            std::cerr << "Could not listen on port " << port << " (is it already in use? try --port 8081)\n";
            return 1;
        }
    } catch (const BankError& e) {
        std::cerr << "Startup failed: " << e.what() << "\n"
                  << "No data was modified. Fix or restore the data files and try again.\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
