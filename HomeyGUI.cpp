#include "HomeyGUI.h"
#include "ConfigManager.h"
#include "HomeExceptions.h"
#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <httplib.h>
#include <cstdint>
#include <SFML/Graphics.hpp>

// --- HELPER FUNCTIONS ---
std::string toLowerUI(std::string str) {
    std::ranges::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
    return str;
}

void clearInputBufferUI() {
    if (std::cin.fail()) std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// --- BUTTON IMPLEMENTATION ---
Button::Button(float x, float y, float w, float h, const std::string& label, const sf::Font& font, bool isSidebar)
    : text(font, label, isSidebar ? 18 : 20), isSidebarBtn(isSidebar)
{
    shape.setPosition({x, y});
    shape.setSize({w, h});

    if (isSidebar) {
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineThickness(0);
        text.setFillColor(Theme::TEXT_SEC);
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({0, bounds.size.y / 2.0f});
        text.setPosition({x + 20.f, y + h / 2.0f});
    } else {
        shape.setFillColor(Theme::SIDEBAR);
        shape.setOutlineThickness(1.f);
        shape.setOutlineColor(sf::Color(60, 60, 70));
        text.setFillColor(Theme::TEXT_MAIN);
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({bounds.size.x / 2.0f + bounds.position.x, bounds.size.y / 2.0f + bounds.position.y});
        text.setPosition({x + w / 2.0f, y + h / 2.0f});
    }
}

bool Button::update(const sf::Vector2f& mousePos, bool isPressed) {
    if (shape.getGlobalBounds().contains(mousePos)) {
        if (isSidebarBtn) {
            shape.setFillColor(Theme::BTN_HOVER);
            text.setFillColor(Theme::ACCENT);
        } else {
            shape.setFillColor(Theme::BTN_HOVER);
            shape.setOutlineColor(Theme::ACCENT);
        }
        if (isPressed) return true;
    } else {
        if (isSidebarBtn) {
            shape.setFillColor(sf::Color::Transparent);
            text.setFillColor(Theme::TEXT_SEC);
        } else {
            shape.setFillColor(Theme::SIDEBAR);
            shape.setOutlineColor(sf::Color(60, 60, 70));
        }
    }
    return false;
}

void Button::draw(sf::RenderWindow& window) const {
    window.draw(shape);
    window.draw(text);
}

// --- HOMEY GUI IMPLEMENTATION ---

HomeyGUI::HomeyGUI() : systemLogic("My Smart Home"), analysisEngine(systemLogic, "rules.txt") {
    // 1280x800 - Rezolutie tipica Laptop/Dashboard
    window.create(sf::VideoMode({1280, 800}), "The Homey - Control Center", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    if (!font.openFromFile("arial.ttf")) {
        throw FileConfigException("arial.ttf (Font missing from assets)");
    }

    currentState = AppState::SETUP;
    serverIp = "127.0.0.1";

    // Setup Buttons
    float cx = 1280.f / 2.f;
    setupButtons.emplace_back(cx - 150, 300, 300, 50, "LOCALHOST (Simulation)", font);
    setupButtons.emplace_back(cx - 150, 370, 300, 50, "RASPBERRY PI (Real)", font);

    // Menu Buttons
    menuButtons.clear();
    menuButtons.emplace_back(cx - 150, 250, 300, 50, "Run Demo System", font);          // Index 0
    menuButtons.emplace_back(cx - 150, 320, 300, 50, "Create System (console)", font); // Index 1
    menuButtons.emplace_back(cx - 150, 390, 300, 50, "Load System From File", font);      // Index 2
    menuButtons.emplace_back(cx - 150, 460, 300, 50, "Exit to Desktop", font);
    // Sidebar Buttons
    sidebarButtons.emplace_back(0, 120, 250, 50, "  DASHBOARD", font, true);
    sidebarButtons.emplace_back(0, 170, 250, 50, "  ANALYTICS", font, true);
    sidebarButtons.emplace_back(0, 220, 250, 50, "  SETTINGS", font, true);
}

void HomeyGUI::run() {
    while (window.isOpen()) {
        handleEvents();
        render();
    }
}
void HomeyGUI::handleEvents() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) window.close();

        else if (const auto* scroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
            if (currentState == AppState::ANALYTICS) {
                // Viteza de scroll
                float scrollSpeed = 30.f;
                analyticsReportScrollY -= scroll->delta * scrollSpeed;

                // Limitare (Clamping) să nu treacă de sus sau jos
                if (analyticsReportScrollY < 0.f) analyticsReportScrollY = 0.f;

                // Calculăm cât spațiu vizibil avem pt text (aprox 400px înălțime cutie)
                float visibleHeight = 400.f;
                float maxScroll = std::max(0.f, analyticsContentHeight - visibleHeight + 50.f); // +50 padding jos
                if (analyticsReportScrollY > maxScroll) analyticsReportScrollY = maxScroll;
            }
        }

        else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape) {
                // Logică de Back extinsă
                if (currentState == AppState::DASHBOARD_ROOM_DETAIL) {
                    currentState = AppState::DASHBOARD_OVERVIEW;
                    selectedRoomIndex = -1;
                }
                else if (currentState == AppState::ANALYTICS) {
                    currentState = AppState::DASHBOARD_OVERVIEW; // Back to Home
                    analyticsReportScrollY = 0.0f; // Reset scroll
                }
                else if (currentState == AppState::DASHBOARD_OVERVIEW) currentState = AppState::MENU;
                else if (currentState == AppState::MENU) currentState = AppState::SETUP;
                else window.close();
            }
        }
    }
}


void HomeyGUI::render() {
    window.clear(Theme::BG);

    if (currentState == AppState::SETUP) drawSetup();
    else if (currentState == AppState::MENU) drawMenu();
    else {
        drawSidebar();
        if (currentState == AppState::DASHBOARD_OVERVIEW) {
            drawOverview();
        }
        else if (currentState == AppState::DASHBOARD_ROOM_DETAIL) {
            auto& allRooms = systemLogic.getRoomList();
            if (selectedRoomIndex >= 0 && selectedRoomIndex < static_cast<int>(allRooms.size())) {
                drawRoomDetail(const_cast<Room*>(&allRooms[selectedRoomIndex]));
            }
        }
        else if (currentState == AppState::ANALYTICS) {
            drawAnalytics();
        }
    }

    window.display();
}

// --- ECRANE SETUP / MENU ---

void HomeyGUI::drawSetup() {
    sf::Text title(font, "SERVER CONFIGURATION", 40);
    title.setFillColor(Theme::ACCENT);
    sf::FloatRect b = title.getLocalBounds();
    title.setOrigin({b.size.x/2, 0});
    title.setPosition({640, 150});
    window.draw(title);

    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    bool click = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

    if (setupButtons[0].update(mouse, click)) { serverIp = "127.0.0.1"; currentState = AppState::MENU; sf::sleep(sf::milliseconds(200)); }
    setupButtons[0].draw(window);

    //ip raspberry pi
    if (setupButtons[1].update(mouse, click)) { serverIp = "192.168.1.140"; currentState = AppState::MENU; sf::sleep(sf::milliseconds(200)); }
    setupButtons[1].draw(window);
}

void HomeyGUI::drawMenu() {
    sf::Text title(font, "THE HOMEY", 60);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(Theme::ACCENT);
    title.setOrigin({title.getLocalBounds().size.x/2, 0});
    title.setPosition({640, 150});
    window.draw(title);

    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    bool click = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

    if (menuButtons[0].update(mouse, click)) {
        initDemoSystem();
        currentState = AppState::DASHBOARD_OVERVIEW;
        sf::sleep(sf::milliseconds(200));
    }

    if (menuButtons[1].update(mouse, click)) {
        runInteractiveConsoleSetup();
        currentState = AppState::DASHBOARD_OVERVIEW;
    }

    if (menuButtons[2].update(mouse, click)) {
        loadSystemFromFileUI();
        if (!systemLogic.getRoomList().empty()) {
            currentState = AppState::DASHBOARD_OVERVIEW;
        }
    }

    if (menuButtons[3].update(mouse, click)) {
        window.close();
    }

    for (auto& btn : menuButtons) {
        btn.draw(window);
    }
}

// --- DASHBOARD UI ---

void HomeyGUI::drawSidebar() {
    sf::RectangleShape sidebar({250.f, 800.f});
    sidebar.setFillColor(Theme::SIDEBAR);
    window.draw(sidebar);

    sf::Text logo(font, "THE\nHOMEY", 28);
    logo.setStyle(sf::Text::Bold);
    logo.setFillColor(Theme::ACCENT);
    logo.setPosition({40.f, 40.f});
    window.draw(logo);

    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    bool click = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    static bool clickReleased = true;
    if (!click) clickReleased = true;

    // --- BUTOANE SIDEBAR ---
    // Dashboard
    if (sidebarButtons[0].update(mouse, click && clickReleased)) {
        currentState = AppState::DASHBOARD_OVERVIEW;
        selectedRoomIndex = -1;
        clickReleased = false;
    }
    sidebarButtons[0].draw(window);

    // Analytics
    if (sidebarButtons[1].update(mouse, click && clickReleased)) {
        currentState = AppState::ANALYTICS;
        analyticsReportScrollY = 0.0f;
        clickReleased = false;
    }
    sidebarButtons[1].draw(window);

    // SETTINGS - Aici apelăm managerul
    if (sidebarButtons[2].update(mouse, click && clickReleased)) {
        runSystemManagerConsole();
        clickReleased = false;
    }
    sidebarButtons[2].draw(window);

    // --- CEAS ȘI DATĂ (DINAMIC) ---
    std::time_t now = std::time(nullptr);
    std::tm* ltm = std::localtime(&now);

    // Ora: HH:MM:SS
    std::stringstream ssTime;
    ssTime << std::put_time(ltm, "%H:%M:%S"); //
    sf::Text txtTime(font, ssTime.str(), 24);
    txtTime.setFillColor(Theme::TEXT_MAIN);
    txtTime.setPosition({40.f, 680.f});
    window.draw(txtTime);

    // Data: DD Mon YYYY
    std::stringstream ssDate;
    ssDate << std::put_time(ltm, "%d %b %Y"); //
    sf::Text txtDate(font, ssDate.str(), 14);
    txtDate.setFillColor(Theme::TEXT_SEC);
    txtDate.setPosition({40.f, 715.f});
    window.draw(txtDate);
}

void HomeyGUI::drawOverview() {
    float startX = 290.f;

    // --- 1. HEADER & GLOBAL HEALTH SCORE ---
    sf::Text title(font, "SYSTEM OVERVIEW", 32);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(Theme::TEXT_MAIN);
    title.setPosition({startX, 40});
    window.draw(title);

    int score = analysisEngine.calculateHealthScore();

    sf::Text scoreTxt(font, "Global Health Score: " + std::to_string(score) + "%", 20);
    scoreTxt.setPosition({startX, 85});

    if (score > 80) scoreTxt.setFillColor(sf::Color(46, 204, 113));
    else if (score > 50) scoreTxt.setFillColor(Theme::WARNING);
    else scoreTxt.setFillColor(Theme::ALERT);
    window.draw(scoreTxt);

    // --- 2. ROOMS GRID ---
    auto& rooms = systemLogic.getRoomList();
    float gridX = startX;
    float gridY = 150.f;

    bool click = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    static bool clickHandled = false;
    if (!click) clickHandled = false;

    int col = 0;
    for (size_t i = 0; i < rooms.size(); ++i) {

        float cardW = 220.f;
        float gap = 20.f;
        std::string roomStatus = "NORMAL";
        sf::Color statusColor = sf::Color(46, 204, 113);
        double maxHazard = 0.0;

        const auto& sensorsInRoom = rooms[i].getSensorList();
        for (const auto& s : sensorsInRoom) {
            double h = s->calculateHazardLevel();
            if (h > maxHazard) maxHazard = h;
        }

        if (maxHazard >= 100.0) {
            roomStatus = "CRITICAL";
            statusColor = Theme::ALERT;
        } else if (maxHazard >= 40.0) {
            roomStatus = "WARNING";
            statusColor = Theme::WARNING;
        } else if (maxHazard >= 25.0) {
            roomStatus = "INFO";
            statusColor = sf::Color(52, 152, 219);
        }

        bool cardClicked = drawRoomCard(gridX + col * (cardW + gap), gridY, rooms[i].getName(), static_cast<int>(sensorsInRoom.size()),roomStatus, statusColor);

        if (cardClicked && click && !clickHandled) {
            selectedRoomIndex = static_cast<int>(i);
            currentState = AppState::DASHBOARD_ROOM_DETAIL;
            clickHandled = true;
        }

        col++;
        if (gridX + (col + 1) * (cardW + gap) > 1280.f) {
            float cardH = 140.f;
            col = 0;
            gridY += cardH + gap;
        }
    }

    // --- 3. ALERTE RECENTE ---
    drawAlertSection(startX, 620.f);
}

void HomeyGUI::drawRoomDetail(const Room* room) {
    if (!room) return;

    // Titlu cameră
    sf::Text title(font, "Room: " + room->getName(), 32);
    title.setFillColor(Theme::TEXT_MAIN);
    title.setPosition({280.f, 40.f});
    window.draw(title);

    const auto& sensors = room->getSensorList();

    float startX = 280.f;
    float startY = 120.f;

    for (size_t i = 0; i < sensors.size(); ++i) {
        float gap = 20.f;
        float cardW = 280.f;
        float cardH = 140.f;

        int columns = 3;
        int row = i / columns;
        int col = i % columns;

        float gx = startX + col * (cardW + gap);
        float gy = startY + row * (cardH + gap);

        if (gy + cardH < window.getSize().y) {
            drawSensorCard(gx, gy, sensors[i]);
        }
    }

    if (sensors.empty()) {
        sf::Text empty(font, "(No sensors in this room)", 18);
        empty.setFillColor(Theme::TEXT_SEC);
        empty.setPosition({startX, startY});
        window.draw(empty);
    }
}

bool HomeyGUI::drawRoomCard(float x, float y, const std::string& name, int sensorCount, const std::string& statusText, sf::Color statusColor) {
    sf::RectangleShape card({220.f, 140.f});
    card.setPosition({x, y});
    card.setFillColor(Theme::CARD_BG);
    card.setOutlineThickness(1.5f);

    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    bool hover = card.getGlobalBounds().contains(mouse);

    // Evidențiem cardul dacă are probleme sau dacă este hover
    if (hover) {
        card.setOutlineColor(Theme::ACCENT);
        card.setFillColor(sf::Color(45, 45, 55));
    } else if (statusText != "NORMAL") {
        card.setOutlineColor(statusColor); // Bordura preia culoarea de stare
    } else {
        card.setOutlineColor(sf::Color(60, 60, 75));
    }
    window.draw(card);

    // Nume cameră
    sf::Text tName(font, name, 18);
    tName.setStyle(sf::Text::Bold);
    tName.setFillColor(Theme::TEXT_MAIN);
    tName.setPosition({x + 15, y + 15});
    window.draw(tName);

    // Număr senzori
    sf::Text tInfo(font, std::to_string(sensorCount) + " Sensors Active", 13);
    tInfo.setFillColor(Theme::TEXT_SEC);
    tInfo.setPosition({x + 15, y + 45});
    window.draw(tInfo);

    // Indicator STATUS (Cel cerut de tine)
    sf::Text tStatus(font, statusText, 14);
    tStatus.setStyle(sf::Text::Bold);
    tStatus.setFillColor(statusColor);
    tStatus.setPosition({x + 15, y + 110});
    window.draw(tStatus);

    return hover;
}

void HomeyGUI::drawSensorCard(float x, float y, const std::shared_ptr<Sensor>& s) {
    if (!s) return;

    std::string type = s->getType();
    double value = s->getValue();
    std::string unit = s->getUnit();
    double hazard = s->calculateHazardLevel();

    // 1. Determinăm culoarea în funcție de logica ta din Sensor.cpp
    sf::Color statusColor = sf::Color(46, 204, 113); // Verde (Default/Safe)

    if (hazard >= 100.0) {
        statusColor = sf::Color(231, 76, 60); // Roșu (Critical)
    } else if (hazard >= 40.0) {
        statusColor = sf::Color(241, 196, 15); // Galben (Warning)
    } else if (hazard >= 25.0) {
        statusColor = sf::Color(52, 152, 219); // Albastru/Cyan (Info)
    }

    // 2. Fundal Card
    sf::RectangleShape card({280.f, 140.f});
    card.setPosition({x, y});
    card.setFillColor(Theme::CARD_BG);
    card.setOutlineThickness(2.f);

    // Bordura devine colorată dacă avem Hazard > 0
    if (hazard > 0) {
        card.setOutlineColor(statusColor);
    } else {
        card.setOutlineColor(sf::Color(255, 255, 255, 20));
    }
    window.draw(card);

    // 3. Titlu Senzor
    sf::Text tType(font, type, 18);
    tType.setStyle(sf::Text::Bold);
    tType.setFillColor(Theme::ACCENT);
    tType.setPosition({x + 20, y + 15});
    window.draw(tType);

    // 4. Afișare Valoare (Numerică sau Status pentru Fum)
    if (type == "fum" || type == "Fum") {
        std::string statusMsg = (value > 0.5) ? "HAZARD" : "CLEAN";
        sf::Text tStatus(font, statusMsg, 24);
        tStatus.setStyle(sf::Text::Bold);
        tStatus.setFillColor(statusColor);

        sf::FloatRect sb = tStatus.getLocalBounds();
        tStatus.setOrigin({sb.size.x / 2.f, sb.size.y / 2.f});
        tStatus.setPosition({x + 140.f, y + 75.f});
        window.draw(tStatus);
    } else {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << value;

        sf::Text tVal(font, ss.str(), 36);
        tVal.setStyle(sf::Text::Bold);
        tVal.setFillColor(statusColor); // Valoarea se colorează conform hazardului

        sf::FloatRect vb = tVal.getLocalBounds();
        tVal.setOrigin({vb.size.x / 2.f, vb.size.y / 2.f});
        tVal.setPosition({x + 140.f, y + 70.f});
        window.draw(tVal);

        sf::Text tUnit(font, unit, 16);
        tUnit.setFillColor(Theme::TEXT_SEC);
        sf::FloatRect ub = tUnit.getLocalBounds();
        tUnit.setOrigin({ub.size.x / 2.f, ub.size.y / 2.f});
        tUnit.setPosition({x + 140.f, y + 105.f});
        window.draw(tUnit);
    }

    if (hazard >= 100.0) {
        sf::Text tCrit(font, "!!! CRITICAL !!!", 12);
        tCrit.setFillColor(statusColor);
        tCrit.setPosition({x + 20, y + 115});
        window.draw(tCrit);
    }
}

void HomeyGUI::drawAlertSection(float x, float y) {
    sf::Text title(font, "ACTIVE ALERTS", 18);
    title.setFillColor(Theme::TEXT_SEC);
    title.setPosition({x, y});
    window.draw(title);

    float currentY = y + 30;
    std::vector<Alert> alerts = analysisEngine.generateAlerts();

    if (alerts.empty()) {
        sf::Text safe(font, "No active alerts. System is stable.", 16);
        safe.setFillColor(sf::Color::Green);
        safe.setPosition({x, currentY});
        window.draw(safe);
    } else {
        for (const auto& alert : alerts) {
            int priority = alert.getPriority();

            sf::Text aText(font, "! " + alert.getMessage(), 16);

            // LOGICA DE CULOARE BAZATĂ PE INT PRIORITY
            if (priority >= 80) {
                aText.setFillColor(Theme::ALERT); // Roșu pentru critic
            } else if (priority >= 40) {
                aText.setFillColor(Theme::WARNING); // Galben pentru mediu
            } else {
                aText.setFillColor(sf::Color::White); // Alb pentru info
            }

            aText.setPosition({x, currentY});
            window.draw(aText);
            currentY += 25;

            // Limitare ca să nu desenăm sub ecran
            if(currentY > 780) break;
        }
    }
}

void HomeyGUI::drawAnalytics() {
    float startX = 290.f;
    float width = 1280.f - startX - 30.f;

    // --- 1. CALCUL STATISTICI ---
    int totalRooms = static_cast<int>(systemLogic.getRoomList().size());
    int totalSensorsReal = 0;

    for (const auto& room : systemLogic.getRoomList()) {
        totalSensorsReal += static_cast<int>(room.getSensorList().size());
    }

    // --- 2. ACTUALIZARE MOTOR ANALIZĂ ---
    int score = analysisEngine.calculateHealthScore();
    std::string reportStr = analysisEngine.generateStatusReport();
    auto alerts = analysisEngine.generateAlerts();
    int activeAlertsCount = static_cast<int>(alerts.size());

    // --- 3. DESENARE HEADER ---
    sf::Text title(font, "SYSTEM DIAGNOSTICS & ANALYTICS", 32);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(Theme::TEXT_MAIN);
    // SFML 3 cere Vector2f explicit
    title.setPosition(sf::Vector2f(startX, 40.f));
    window.draw(title);

    // --- 4. HEALTH SCORE CARD ---
    float cardY = 100.f;
    float scoreCardW = 350.f;

    sf::RectangleShape scoreCard(sf::Vector2f(scoreCardW, 220.f));
    scoreCard.setPosition(sf::Vector2f(startX, cardY));
    scoreCard.setFillColor(Theme::CARD_BG);
    scoreCard.setOutlineThickness(1.5f);

    sf::Color scoreColor = sf::Color(46, 204, 113); // Verde
    if (score < 50) scoreColor = Theme::ALERT;      // Roșu
    else if (score < 80) scoreColor = Theme::WARNING; // Galben

    scoreCard.setOutlineColor(scoreColor);
    window.draw(scoreCard);

    sf::Text scoreLbl(font, "OVERALL HEALTH SCORE", 16);
    scoreLbl.setFillColor(Theme::TEXT_SEC);
    scoreLbl.setPosition(sf::Vector2f(startX + 25.f, cardY + 25.f));
    window.draw(scoreLbl);

    sf::Text scoreVal(font, std::to_string(score), 90);
    scoreVal.setStyle(sf::Text::Bold);
    scoreVal.setFillColor(scoreColor);
    sf::FloatRect sb = scoreVal.getLocalBounds();
    scoreVal.setOrigin(sf::Vector2f(sb.size.x/2.f, sb.size.y/2.f));
    scoreVal.setPosition(sf::Vector2f(startX + scoreCardW/2.f, cardY + 120.f));
    window.draw(scoreVal);

    sf::Text scoreSuffix(font, "/ 100", 24);
    scoreSuffix.setFillColor(Theme::TEXT_SEC);
    scoreSuffix.setPosition(sf::Vector2f(startX + scoreCardW/2.f + sb.size.x/2.f + 10.f, cardY + 130.f));
    window.draw(scoreSuffix);

    float statsX = startX + scoreCardW + 30.f;
    float statsW = width - scoreCardW - 30.f;
    sf::RectangleShape statsCard(sf::Vector2f(statsW, 220.f));
    statsCard.setPosition(sf::Vector2f(statsX, cardY));
    statsCard.setFillColor(Theme::CARD_BG);
    statsCard.setOutlineColor(sf::Color(60,60,70));
    statsCard.setOutlineThickness(1.f);
    window.draw(statsCard);

    sf::Text statsLbl(font, "SYSTEM STATISTICS", 16);
    statsLbl.setFillColor(Theme::TEXT_SEC);
    statsLbl.setPosition(sf::Vector2f(statsX + 25.f, cardY + 25.f));
    window.draw(statsLbl);

    auto drawMiniStat = [&](float sx, float sy, const std::string& label, int val, sf::Color valColor) {
        sf::Text vText(font, std::to_string(val), 36);
        vText.setStyle(sf::Text::Bold);
        vText.setFillColor(valColor);
        vText.setPosition(sf::Vector2f(sx, sy));
        window.draw(vText);

        sf::Text lText(font, label, 14);
        lText.setFillColor(Theme::TEXT_SEC);
        lText.setPosition(sf::Vector2f(sx, sy + 45.f));
        window.draw(lText);
    };

    float statColW = statsW / 3.0f;
    float statStartY = cardY + 80.f;

    drawMiniStat(statsX + 30, statStartY, "Total Rooms", totalRooms, Theme::TEXT_MAIN);
    drawMiniStat(statsX + 30 + statColW, statStartY, "Active Sensors", totalSensorsReal, Theme::ACCENT);
    drawMiniStat(statsX + 30 + statColW*2, statStartY, "Active Alerts", activeAlertsCount, (activeAlertsCount > 0 ? Theme::ALERT : sf::Color::Green));

    float reportY = cardY + 220.f + 30.f;
    float reportH = 800.f - reportY - 30.f;

    sf::Text reportLbl(font, "DETAILED STATUS REPORT & LOGS", 18);
    reportLbl.setFillColor(Theme::TEXT_MAIN);
    reportLbl.setPosition(sf::Vector2f(startX, reportY));
    window.draw(reportLbl);

    sf::RectangleShape termBg(sf::Vector2f(width, reportH));
    termBg.setPosition(sf::Vector2f(startX, reportY + 30.f));
    termBg.setFillColor(sf::Color(10, 10, 15));
    termBg.setOutlineColor(Theme::ACCENT);
    termBg.setOutlineThickness(1.f);
    window.draw(termBg);

    std::string fullReportStr = reportStr;
    if (!alerts.empty()) {
        fullReportStr += "\n\n[CRITICAL ALERTS LOG]\n";
        for (const auto& a : alerts) {
             fullReportStr += ">>> [ALERT] Prio:" + std::to_string(a.getPriority()) + " | " + a.getMessage() + "\n";
        }
    }

    sf::Text reportText(font, fullReportStr, 16);
    reportText.setFillColor(sf::Color(200, 220, 200));
    reportText.setLineSpacing(1.2f);

    reportText.setPosition(sf::Vector2f(0.f, 0.f));

    sf::View originalView = window.getView();

    sf::Vector2u winSize = window.getSize();
    float vpX = startX / static_cast<float>(winSize.x);
    float vpY = (reportY + 30.f) / static_cast<float>(winSize.y);
    float vpW = width / static_cast<float>(winSize.x);
    float vpH = reportH / static_cast<float>(winSize.y);

    sf::View scrollView;

    scrollView.setSize(sf::Vector2f(width, reportH));

    scrollView.setViewport(sf::FloatRect({vpX, vpY}, {vpW, vpH}));
    scrollView.setCenter(sf::Vector2f(width / 2.f, reportH / 2.f + analyticsReportScrollY));

    window.setView(scrollView);
    window.draw(reportText);

    window.setView(originalView);

    analyticsContentHeight = reportText.getLocalBounds().size.y;
}
// --- BACKEND LOGIC INTEGRATION ---

void HomeyGUI::initDemoSystem() {
    std::cout << "[GUI] Initializing Demo System...\n";
    systemLogic = HomeSystem("Demo Smart Home");

    double baseTemp = getLiveValueFromPi("Temperatura");
    double baseHum = getLiveValueFromPi("Umiditate");
    double baseLux = getLiveValueFromPi("Lumina");
    double baseCO2 = getLiveValueFromPi("CO2");
    double basePM = getLiveValueFromPi("PM2.5");
    double baseFum = getLiveValueFromPi("Fum");
    double baseTVOC = getLiveValueFromPi("TVOC");
    double baseCO = getLiveValueFromPi("CO");
    double baseSunet = getLiveValueFromPi("Sunet");

    std::cout << "[INIT] Data fetched. Building rooms...\n";

    HomeSystem home("The Homey (Live Demo)");

    // aici avem datele reale (in living se afla raspberry pi-ul)
    Room livingRoom("Living Room");
    livingRoom.addSensor(std::make_shared<temperatureSensor>(baseTemp));
    livingRoom.addSensor(std::make_shared<humiditySensor>(baseHum));
    livingRoom.addSensor(std::make_shared<toxicGasSensor>("TVOC", baseTVOC, "ppb"));
    livingRoom.addSensor(std::make_shared<toxicGasSensor>("CO2", baseCO2, "ppm"));
    livingRoom.addSensor(std::make_shared<particleSensor>(basePM));
    livingRoom.addSensor(std::make_shared<lightSensor>(baseLux));
    livingRoom.addSensor(std::make_shared<smokeSensor>(baseFum));
    livingRoom.addSensor(std::make_shared<toxicGasSensor>("CO", baseCO, "ppm"));
    livingRoom.addSensor(std::make_shared<soundSensor>(baseSunet));


    Room bedroom("Bedroom");
    bedroom.addSensor(std::make_shared<temperatureSensor>(applySimulationOffset(baseTemp, "Temperatura")));
    bedroom.addSensor(std::make_shared<humiditySensor>(applySimulationOffset(baseHum, "Umiditate")));
    bedroom.addSensor(std::make_shared<toxicGasSensor>("TVOC", applySimulationOffset(baseTVOC, "TVOC"), "ppb"));
    bedroom.addSensor(std::make_shared<toxicGasSensor>("CO2", applySimulationOffset(baseCO2, "CO2"), "ppm"));
    bedroom.addSensor(std::make_shared<particleSensor>(applySimulationOffset(basePM, "PM2.5")));
    bedroom.addSensor(std::make_shared<lightSensor>(applySimulationOffset(baseLux, "Lumina")));
    bedroom.addSensor(std::make_shared<smokeSensor>(baseFum)); // Fumul de obicei e la fel (0)
    bedroom.addSensor(std::make_shared<toxicGasSensor>("CO", baseCO, "ppm"));
    bedroom.addSensor(std::make_shared<soundSensor>(applySimulationOffset(baseSunet, "Sunet") - 10.0));


    Room kitchen("Kitchen");
    // aici adaugam un offset mai mare - in bucatarie e mai cald
    double kitchenTemp = applySimulationOffset(baseTemp, "Temperatura") + 2.0;

    kitchen.addSensor(std::make_shared<temperatureSensor>(kitchenTemp));
    kitchen.addSensor(std::make_shared<humiditySensor>(applySimulationOffset(baseHum, "Umiditate")));
    kitchen.addSensor(std::make_shared<toxicGasSensor>("TVOC", applySimulationOffset(baseTVOC, "TVOC"), "ppb"));
    kitchen.addSensor(std::make_shared<toxicGasSensor>("CO2", applySimulationOffset(baseCO2, "CO2"), "ppm"));
    kitchen.addSensor(std::make_shared<particleSensor>(applySimulationOffset(basePM, "PM2.5")));
    kitchen.addSensor(std::make_shared<lightSensor>(applySimulationOffset(baseLux, "Lumina")));
    kitchen.addSensor(std::make_shared<smokeSensor>(baseFum));
    kitchen.addSensor(std::make_shared<toxicGasSensor>("CO", baseCO, "ppm"));
    kitchen.addSensor(std::make_shared<soundSensor>(applySimulationOffset(baseSunet, "Sunet") + 15.0));

    Room garage("Garage");
    // gol

    systemLogic.addRoom(livingRoom);
    systemLogic.addRoom(bedroom);
    systemLogic.addRoom(kitchen);
    systemLogic.addRoom(garage);
}

void HomeyGUI::runInteractiveConsoleSetup() {
    // 1. Ascundem fereastra grafică pentru a ne concentra pe consolă
    window.setVisible(false);

    std::cout << "\n========================================\n";
    std::cout << "   INTERACTIVE CONSOLE SETUP MODE\n";
    std::cout << "========================================\n";

    // 2. Setare Nume Sistem
    std::string sysName;
    std::cout << "Enter new system name: ";
    clearInputBufferUI();
    std::getline(std::cin, sysName);

    // Re-inițializăm sistemul din GUI cu numele nou
    systemLogic = HomeSystem(sysName);

    // Motor de analiză temporar pentru afișarea în consolă
    AnalysisEngine consoleEngine(systemLogic, "rules.txt");

    bool running = true;
    while(running) {
        std::cout << "\n--- System: " << sysName  << " ---\n";
        std::cout << "1. Add Room\n";
        std::cout << "2. Add Sensor (to a Room)\n";
        std::cout << "3. Run Analysis (Console Report)\n";
        std::cout << "4. Display Full System Structure\n";
        std::cout << "5. Save system to file\n";
        std::cout << "0. FINISH & RETURN TO DASHBOARD\n";
        std::cout << "Select option: ";

        int option = 0;
        if (!(std::cin >> option)) {
            clearInputBufferUI();
            continue;
        }

        try {
            switch(option) {
            case 1: {
                std::string roomName;
                std::cout << "Enter room name: ";
                clearInputBufferUI();
                std::getline(std::cin, roomName);
                systemLogic.addRoom(Room(roomName));
                std::cout << "Room '" << roomName << "' added.\n";
                break;
            }
            case 2: {
                std::string roomName;
                std::cout << "Enter room name to add sensor to: ";
                clearInputBufferUI();
                std::getline(std::cin, roomName);

                // Căutăm în sistemul curent
                Room* room = systemLogic.findRoomByName(roomName);
                if (room == nullptr) {
                    throw RoomNotFoundException(roomName);
                }

                std::cout << "Select sensor type:\n";
                std::cout << "1. Temperatura\n2. Umiditate\n3. Lumina\n4. CO2\n5. PM2.5\n6. Fum\n7. TVOC\n8. CO\n9. Sunet\n10. ADD ALL SENSORS (Full Package)\n";
                std::cout << "Choice: ";
                int sOpt;
                std::cin >> sOpt;

                std::vector<int> sensorsToAdd;
                if (sOpt == 10) {
                    sensorsToAdd = {1, 2, 3, 4, 5, 6, 7, 8, 9};
                    std::cout << "[INFO] Selected ALL sensors package.\n";
                } else if (sOpt >= 1 && sOpt <= 9) {
                    sensorsToAdd.push_back(sOpt);
                } else {
                    std::cout << "Invalid selection.\n";
                    break;
                }

                std::cout << "\nData Source:\n1. Manual Input\n2. Live from Server (" << serverIp << ")\nChoice: ";
                int sourceOpt;
                std::cin >> sourceOpt;

                bool applyVariation = false;
                if (sourceOpt == 2) {
                    std::cout << "Is this the MAIN room (exact values)? (1-Yes, 0-No/Apply Variation): ";
                    int isMainRoom;
                    std::cin >> isMainRoom;

                    if (isMainRoom == 0) {
                        applyVariation = true;
                        std::cout << "[INFO] Values will be simulated based on Real Pi data (Variations applied).\n";
                    }
                }

                for (int sensorIndex : sensorsToAdd) {
                    std::string type;
                    switch(sensorIndex) {
                        case 1: type = "Temperatura"; break;
                        case 2: type = "Umiditate"; break;
                        case 3: type = "Lumina"; break;
                        case 4: type = "CO2"; break;
                        case 5: type = "PM2.5"; break;
                        case 6: type = "Fum"; break;
                        case 7: type = "TVOC"; break;
                        case 8: type = "CO"; break;
                        case 9: type = "Sunet"; break;
                        default: type = "Unknown"; break;
                    }

                    double val = 0.0;
                    if (sourceOpt == 2) {
                        double realVal = getLiveValueFromPi(type);

                        if (applyVariation) {
                            val = applySimulationOffset(realVal, type);
                            std::cout << "   -> Real: " << realVal << " | Simulated: " << val << "\n";
                        } else {
                            val = realVal;
                        }
                    } else {
                        std::cout << "Enter value for " << type << ": ";
                        std::cin >> val;
                    }

                    bool found = false;
                    for (const auto& sensor : room->getSensorList()) {
                        if (sensor->getType() == type) {
                            sensor->updateValue(val);
                            std::cout << "[UPDATE] Sensor " << type << " updated to: " << val << "\n";
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        Sensor* newSensor = nullptr;
                        try {
                            switch(sensorIndex) {
                                case 1: newSensor = new temperatureSensor(val); break;
                                case 2: newSensor = new humiditySensor(val); break;
                                case 3: newSensor = new lightSensor(val); break;
                                case 4: newSensor = new toxicGasSensor("CO2", val, "ppm"); break;
                                case 5: newSensor = new particleSensor(val); break;
                                case 6: newSensor = new smokeSensor(val); break;
                                case 7: newSensor = new toxicGasSensor("TVOC", val, "ppb"); break;
                                case 8: newSensor = new toxicGasSensor("CO", val, "ppm"); break;
                                case 9: newSensor = new soundSensor(val); break;
                                default: std::cerr << "[WARNING] Unknown sensor index: " << sensorIndex << ". Skipping.\n"; break;
                            }
                        } catch (...) {
                            std::cout << "[ERROR] Error creating " << type << "\n";
                            delete newSensor;
                            continue;
                        }

                        if (newSensor != nullptr) {
                            room->addSensor(std::shared_ptr<Sensor>(newSensor));
                            std::cout << "[SUCCESS] Added " << type << ": " << val << "\n";
                        }
                    }
                }
                std::cout << "\n[DONE] Finished adding sensors to " << roomName << ".\n";
                break;
            }
            case 3: { // Analysis
                AnalysisEngine tempEngine(systemLogic, "rules.txt");
                std::cout << "\n--- ANALYSIS REPORT ---\n";
                std::cout << tempEngine.generateStatusReport();
                std::cout << "System Health Score: " << tempEngine.calculateHealthScore() << "/100\n";

                auto alerts = tempEngine.generateAlerts();
                if (alerts.empty()) std::cout << "No alerts. System stable.\n";
                else {
                    std::cout << "\n!!! ALERTS DETECTED !!!\n";
                    for(const auto& a : alerts) std::cout << "  -> " << a << "\n";
                }
                break;
            }
            case 4: { // Display
                std::cout << "\n--- FULL SYSTEM STRUCTURE ---\n";
                std::cout << systemLogic;
                break;
            }
            case 5: { // Save
                std::string filename;
                std::cout << "Enter filename (e.g., config.txt): ";
                clearInputBufferUI();
                std::getline(std::cin, filename);
                if(filename.empty()) filename = "config.txt";

                ConfigManager::saveSystemToFile(filename, systemLogic);
                break;
            }
            case 0: {
                std::cout << "Returning to Dashboard...\n";
                running = false;
                break;
            }
            default:
                std::cout << "Invalid option!\n";
                break;
            }
        }
        catch (const std::exception& e) {
            std::cout << "\n[!] ERROR: " << e.what() << "\n";
        }
    }

    std::cout << "\nExiting Console Setup... Returning to Dashboard.\n";

    window.setVisible(true);
}

void HomeyGUI::loadSystemFromFileUI() {
    window.setVisible(false);
    std::cout << "\n--- LOAD FROM FILE ---\n";
    std::cout << "Filename: ";

    std::string fname;
    std::getline(std::cin, fname);

    if (fname.empty()) {
        std::getline(std::cin, fname);
    }

    if (!fname.empty() && fname.back() == '\r') {
        fname.pop_back();
    }

    if(fname.empty()) fname = "config.txt";

    try {
        systemLogic = ConfigManager::loadSystemFromFile(fname);
        std::cout << "[OK] System loaded successfully.\n";
    } catch (const std::exception& e) {
        std::cout << "[ERROR] " << e.what() << "\n";
        sf::sleep(sf::milliseconds(2000));
    }
    window.setVisible(true);
}

void HomeyGUI::runSystemManagerConsole() {
    window.setVisible(false);
    while (true) {
        std::cout << "\n========== SETTINGS: " << systemLogic.getSystemName() << " ==========\n";
        std::cout << "1. Add Room\n";
        std::cout << "2. Delete Room\n";
        std::cout << "3. Add/Update Sensors\n";
        std::cout << "4. Delete Sensor\n";
        std::cout << "5. Save System to File\n";
        std::cout << "0. RETURN TO DASHBOARD\n";
        std::cout << "Select option: ";

        int option;
        if (!(std::cin >> option)) { clearInputBufferUI(); continue; }
        if (option == 0) break;

        try {
            switch (option) {
                case 1: { // ADD ROOM
                    std::cout << "Enter new room name: ";
                    std::string rName; clearInputBufferUI(); std::getline(std::cin, rName);
                    systemLogic.addRoom(Room(rName));
                    std::cout << "[OK] Room added.\n";
                    break;
                }

                case 2: { // DELETE ROOM
                    std::cout << "\n--- Available Rooms ---\n";
                    const auto& rooms = systemLogic.getRoomList();
                    if (rooms.empty()) { std::cout << "No rooms to delete.\n"; break; }
                    for (const auto& r : rooms) std::cout << " - " << r.getName() << "\n";

                    std::cout << "Enter room name to DELETE: ";
                    std::string rName; clearInputBufferUI(); std::getline(std::cin, rName);
                    systemLogic.removeRoom(Room(rName));
                    std::cout << "[OK] Room deleted successfully.\n";
                    break;
                }

                case 3: { // ADD / UPDATE SENSOR
                    const auto& rooms = systemLogic.getRoomList();
                    if (rooms.empty()) {
                        std::cout << "[!] Nu exista camere. Creeaza una mai intai!\n";
                        break;
                    }

                    std::cout << "\n--- Camere Disponibile ---\n";
                    for (const auto& r : rooms) std::cout << " -> " << r.getName() << "\n";

                    std::cout << "Selecteaza camera: ";
                    std::string rName; clearInputBufferUI(); std::getline(std::cin, rName);
                    Room* room = systemLogic.findRoomByName(rName);
                    if (!room) {
                        throw RoomNotFoundException(rName);
                    }

                    std::cout << "\n[1.Temp | 2.Hum | 3.Light | 4.CO2 | 5.PM2.5 | 6.Smoke | 7.TVOC | 8.CO | 9.Sound | 10.ALL]\n";
                    std::cout << "Optiune: ";
                    int sOpt; std::cin >> sOpt;

                    std::vector<int> toProcess;
                    if (sOpt == 10) toProcess = {1,2,3,4,5,6,7,8,9};
                    else if (sOpt >= 1 && sOpt <= 9) toProcess.push_back(sOpt);
                    else { std::cout << "[!] Optiune invalida.\n"; break; }

                    std::cout << "Sursa date: 1.Manual | 2.Live: ";
                    int src; std::cin >> src;

                    for (int idx : toProcess) {
                        std::string type;
                        if(idx==1) type="Temperatura";
                        else if(idx==2) type="Umiditate";
                        else if(idx==3) type="Lumina";
                        else if(idx==4) type="CO2";
                        else if(idx==5) type="PM2.5";
                        else if(idx==6) type="Fum";
                        else if(idx==7) type="TVOC";
                        else if(idx==8) type="CO";
                        else if(idx==9) type="Sunet";

                        double val = 0.0;
                        if (src == 2) {
                            val = getLiveValueFromPi(type);
                        } else {
                            std::cout << "Valoare pentru " << type << ": ";
                            std::cin >> val;
                        }

                        // Verificam daca senzorul exista deja pentru UPDATE
                        bool updated = false;
                        for (auto& s : room->getSensorList()) {
                            if (s->getType() == type) {
                                s->updateValue(val);
                                std::cout << "[UPDATE] " << type << " -> " << val << "\n";
                                updated = true;
                                break;
                            }
                        }

                        // Daca nu exista, il cream si il adaugam (ADD)
                        if (!updated) {
                            Sensor* newS = nullptr;
                            if(idx==1) newS = new temperatureSensor(val);
                            else if(idx==2) newS = new humiditySensor(val);
                            else if(idx==3) newS = new lightSensor(val);
                            else if(idx==4) newS = new toxicGasSensor("CO2", val, "ppm");
                            else if(idx==5) newS = new particleSensor(val);
                            else if(idx==6) newS = new smokeSensor(val);
                            else if(idx==7) newS = new toxicGasSensor("TVOC", val, "ppb");
                            else if(idx==8) newS = new toxicGasSensor("CO", val, "ppm");
                            else if(idx==9) newS = new soundSensor(val);

                            if (newS) {
                                room->addSensor(std::shared_ptr<Sensor>(newS));
                                std::cout << "[ADD] " << type << " a fost adaugat.\n";
                            } else {
                                std::cout << "[!] Eroare la crearea senzorului " << type << "\n";
                            }
                        }
                    }
                    std::cout << "\n[GATA] Procesare finalizata pentru " << rName << ".\n";
                    break;
                    }

                case 4: { // DELETE SENSOR
                    std::cout << "\n--- Select Room ---\n";
                    for (const auto& r : systemLogic.getRoomList()) std::cout << " - " << r.getName() << "\n";

                    std::cout << "Room name: ";
                    std::string rName; clearInputBufferUI(); std::getline(std::cin, rName);


                    if (Room* room = systemLogic.findRoomByName(rName)) {
                        std::cout << "\n--- Sensors in " << rName << " ---\n";
                        const auto& sList = room->getSensorList();
                        if (sList.empty()) { std::cout << "Room is empty.\n"; break; }
                        for (const auto& s : sList) std::cout << " - " << s->getType() << " (Val: " << s->getValue() << ")\n";

                        std::cout << "Sensor type to delete: ";
                        std::string sType; std::cin >> sType;

                        std::shared_ptr<Sensor> toDel = nullptr;
                        for (auto& s : sList) if (s->getType() == sType) { toDel = s; break; }

                        if (toDel) {
                            room->removeSensor(toDel);
                            std::cout << "[OK] Sensor removed.\n";
                        } else std::cout << "[!] Sensor not found.\n";
                    }
                    break;
                }

                case 5: { // SAVE
                    std::cout << "Filename: ";
                    std::string fn; clearInputBufferUI(); std::getline(std::cin, fn);
                    ConfigManager::saveSystemToFile(fn, systemLogic);
                    std::cout << "[OK] Saved to " << fn << "\n";
                    break;
                }

                default: {
                    std::cout << "[!] Invalid option (" << option << "). Try again.\n";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    break;
                }
            }
        } catch (const std::exception& e) {
            std::cout << "[ERROR] " << e.what() << "\n";
        }
    }
    window.setVisible(true);
}

void HomeyGUI::updateSensors() const {
    auto& rooms = systemLogic.getRoomList();
    for(auto& r : rooms) {
        bool isLiving = (r.getName() == "Living Room");
        bool isKitchen = (r.getName() == "Kitchen");

        for(auto& s : r.getSensorList()) {
            std::string type = s->getType();
            double val = getLiveValueFromPi(type);

            // Logica de simulare/offset
            if (isLiving) {
                // Living are date "reale" (sau baza)
            } else if (isKitchen && type == "Temperatura") {
                val = applySimulationOffset(val, type) + 2.0;
            } else {
                val = applySimulationOffset(val, type);
            }

            // Asigura-te ca valoarea e pozitiva
            if(val < 0) val = 0;
            s->updateValue(val);
        }
    }
}


double HomeyGUI::getLiveValueFromPi(const std::string& sensorType) const{
    std::string typeNormalizat = toLowerUI(sensorType);
    std::string jsonKey;

    if (typeNormalizat == "temperatura") jsonKey = "temperature";
    else if (typeNormalizat == "umiditate") jsonKey = "humidity";
    else if (typeNormalizat == "lumina") jsonKey = "lux";
    else if (typeNormalizat == "co2") jsonKey = "mq135_co2";
    else if (typeNormalizat == "pm2.5") jsonKey = "pm2_5";
    else if (typeNormalizat == "fum") jsonKey = "fum";
    else if (typeNormalizat == "tvoc") jsonKey = "tvoc";
    else if (typeNormalizat == "co") jsonKey = "co";
    else if (typeNormalizat == "sunet") jsonKey = "sunet";
    else return 0.0;

    httplib::Client cli(serverIp, 5000);
    cli.set_connection_timeout(0, 100000);

    auto res = cli.Get("/sensors");

    if (res && res->status == 200) {
        try {
            return extractNestedValue(res->body, jsonKey);
        } catch (const InvalidDataSensorException& e) {
            std::cerr << "[Network Error] " << e.what() << "\n";
            return 0.0;
        }
    }

    std::cerr << "[WARNING] Could not fetch live data from Pi (Status: "
              << (res ? std::to_string(res->status) : "Unreachable") << ")\n";

    return 0.0;
}
double HomeyGUI::extractNestedValue(const std::string& json, const std::string& sensorName) {
    std::string keySearch = "\"" + sensorName + "\"";
    size_t keyPos = json.find(keySearch);

    if (keyPos == std::string::npos) {
        throw InvalidDataSensorException("JSON Key missing: " + sensorName);
    }

    size_t valueLabelPos = json.find("\"valoare\"", keyPos);
    if (valueLabelPos == std::string::npos || valueLabelPos - keyPos > 100) {
        throw InvalidDataSensorException("Malformed JSON structure for: " + sensorName);
    }

    size_t colonPos = json.find(':', valueLabelPos);
    size_t endPos = json.find_first_of(",}", colonPos);
    std::string valStr = json.substr(colonPos + 1, endPos - colonPos - 1);

    try {
        return std::stod(valStr);
    } catch (...) {
        throw InvalidDataSensorException("Cannot convert value to double for: " + sensorName);
    }
}

double HomeyGUI::applySimulationOffset(double baseValue, const std::string& type) {
    if (type == "Fum") return baseValue;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(-1.0, 1.0);
    double randomFactor = dis(gen);
    double offset = 0.0;
    if (type == "Temperatura") offset = randomFactor * 3.0;
    else if (type == "Umiditate") offset = randomFactor * 10.0;
    else if (type == "Lumina") offset = randomFactor * 150.0;
    else if (type == "CO2") offset = randomFactor * 50.0;
    else offset = randomFactor * 1.0;
    double finalValue = std::round((baseValue + offset) * 10.0) / 10.0;
    if (finalValue < 0.0) return 0.0;
    if (type == "Umiditate" && finalValue > 100.0) return 100.0;
    return finalValue;
}

