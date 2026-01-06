#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>

#include "HomeSystem.h"
#include "Sensor.h"
#include "Room.h"
#include "AnalysisEngine.h"


namespace Theme {
    constexpr sf::Color BG(18, 18, 24);             // Deep Dark Blue/Black
    constexpr sf::Color SIDEBAR(25, 25, 35);        // Slightly lighter
    constexpr sf::Color CARD_BG(30, 30, 40);        // Card background
    constexpr sf::Color ACCENT(0, 229, 255);        // Cyan Neon
    constexpr sf::Color WARNING(255, 214, 0);       // Yellow
    constexpr sf::Color ALERT(255, 61, 0);          // Red/Orange Neon
    constexpr sf::Color TEXT_MAIN(255, 255, 255);   // White
    constexpr sf::Color TEXT_SEC(150, 150, 160);    // Grey text
    constexpr sf::Color BTN_HOVER(45, 45, 55);
}

// --- BUTTON CLASS ---
class Button {
public:
    Button(float x, float y, float w, float h, const std::string& label, const sf::Font& font, bool isSidebar = false);
    bool update(const sf::Vector2f& mousePos, bool isPressed);
    void draw(sf::RenderWindow& window) const;
private:
    sf::RectangleShape shape;
    sf::Text text;
    bool isSidebarBtn;
};

// --- MAIN GUI CLASS ---
class HomeyGUI {
public:
    HomeyGUI();
    void run();

private:
    HomeSystem systemLogic;
    AnalysisEngine analysisEngine;
    // Stările Aplicației
    enum class AppState { SETUP, MENU, DASHBOARD_OVERVIEW, DASHBOARD_ROOM_DETAIL, ANALYTICS, EXIT };

    void handleEvents();
    void render();
    void drawAnalytics();

    // Ecrane Principale
    void drawSetup();       // Ecran 1: IP Setup
    void drawMenu();        // Ecran 2: Meniu Principal

    // Dashboard Components
    void drawSidebar();     // Meniul din stanga
    void drawOverview();    // Ecranul "Home" (Camere + Alerte)
    void drawRoomDetail(const Room* room);  // Ecranul "Detalii Camera"

    // UI Helpers
    bool drawRoomCard(float x, float y, const std::string& name, int sensorCount, const std::string& statusText, sf::Color statusColor);
    void drawSensorCard(float x, float y, const std::shared_ptr<Sensor>& s);
    void drawAlertSection(float x, float y);
    void runSystemManagerConsole();

    // --- INTEGRATED BACKEND LOGIC ---
    void initDemoSystem();
    void runInteractiveConsoleSetup();

    void updateSensors() const;
    double getLiveValueFromPi(const std::string& sensorType) const;

    void loadSystemFromFileUI();

    static double applySimulationOffset(double baseValue, const std::string& type);
    static double extractNestedValue(const std::string& json, const std::string& sensorName);

    // Variabile SFML
    sf::RenderWindow window;
    sf::Font font;
    AppState currentState;

    // Navigation State
    int selectedRoomIndex = -1;
    float analyticsReportScrollY = 0.0f;
    float analyticsContentHeight = 0.0f;

    // Backend System
    std::string serverIp;


    // Buttons
    std::vector<Button> setupButtons;
    std::vector<Button> menuButtons;
    std::vector<Button> sidebarButtons;
};