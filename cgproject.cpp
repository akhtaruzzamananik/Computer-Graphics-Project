/*
   Metro Rail Simulation (C++ / OpenGL GLUT) - Single File Project
   --------------------------------------------------------------
   Updates:
     - Fixed signboard width so building names (RASG-1, YKSG-1, RASG-2) stay inside the blue box.
     - Added a blue rectangular signboard above each building with their names.
     - Station sign reads "DIU METRO STATION".
     - Train vertically adjusted to sit exactly on the front rail line.
     - Wheels are solid completely round and black.
     - Fixed background bleed.
     - Gate slides to ONE SIDE (right) and is COMPLETELY WHITE when active.
     - RED body color at all other times.

   Controls: ESC -> Exit
*/

#include <GL/glut.h>
#include <cmath>
#include <algorithm>
#include <cstring>

// --------------------------- Canvas / Timing ---------------------------
static const int W = 1000;
static const int H = 600;
static const int TIMER_MS = 16;   // ~60 FPS (to control the frame rate)
static const float DT = 0.016f;

// --------------------------- Utility ---------------------------
static inline int iround(float x) { return (int)std::lround(x); }

static void plotPoint(int x, int y) {
    glVertex2i(x, y);
}

// --------------------------- DDA Line Algorithm ---------------------------
// Function to draw a line using the DDA algorithm
static void lineDDA(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1, dy = y2 - y1;
    float steps = std::max(std::fabs(dx), std::fabs(dy));
    if (steps < 1.0f) steps = 1.0f;

    float xInc = dx / steps, yInc = dy / steps;
    float x = x1, y = y1;

    for (int i = 0; i <= (int)steps; i++) {
        plotPoint(iround(x), iround(y));
        x += xInc; y += yInc;
    }
}

// --------------------------- Bresenham Line Algorithm ---------------------------
// Function to draw a line using Bresenham's algorithm
static void lineBresenham(int x1, int y1, int x2, int y2) {
    int dx = std::abs(x2 - x1), dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1, sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        plotPoint(x1, y1);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 <  dx) { err += dx; y1 += sy; }
    }
}

// --------------------------- Midpoint Circle Algorithm ---------------------------
// Midpoint algorithm for drawing a circle
static void circleMidpoint(int xc, int yc, int r) {
    int x = 0, y = r, d = 1 - r;
    auto plot8 = [&](int px, int py) {
        plotPoint(xc + px, yc + py); plotPoint(xc - px, yc + py);
        plotPoint(xc + px, yc - py); plotPoint(xc - px, yc - py);
        plotPoint(xc + py, yc + px); plotPoint(xc - py, yc + px);
        plotPoint(xc + py, yc - px); plotPoint(xc - py, yc - px);
    };

    plot8(x, y);
    while (x < y) {
        x++;
        if (d < 0) d += 2 * x + 1;
        else { y--; d += 2 * (x - y) + 1; }
        plot8(x, y);
    }
}

// --------------------------- Drawing Helpers ---------------------------
// Shortcut function to set the color
static void setColor(float r, float g, float b) { glColor3f(r, g, b); }

// Function to draw a filled (solid) rectangle
static void rectFilled(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x + w, y);
    glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

// Function to draw an outlined rectangle
static void rectOutline(float x, float y, float w, float h) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y); glVertex2f(x + w, y);
    glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

// Function to render text on the screen
static void renderText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
}

static void rectOutlineBresenham(int x, int y, int w, int h) {
    glBegin(GL_POINTS);
    lineBresenham(x, y, x + w, y);
    lineBresenham(x + w, y, x + w, y + h);
    lineBresenham(x + w, y + h, x, y + h);
    lineBresenham(x, y + h, x, y);
    glEnd();
}

static void rectOutlineDDA(int x, int y, int w, int h) {
    glBegin(GL_POINTS);
    lineDDA((float)x, (float)y, (float)(x + w), (float)y);
    lineDDA((float)(x + w), (float)y, (float)(x + w), (float)(y + h));
    lineDDA((float)(x + w), (float)(y + h), (float)x, (float)(y + h));
    lineDDA((float)x, (float)(y + h), (float)x, (float)y);
    glEnd();
}

// --------------------------- Scene Objects (Always Day Mode) ---------------------------
static void drawBuildings() {
    // Configuration for 5 buildings stored in an array.
    // x = horizontal position, y = vertical position, w = width, h = height, s = scale, name = building name.
    // All buildings will be drawn via a loop using this single array.
    struct B { float x, y, w, h, s; const char* name; };
    B bs[] = {
        {  40, 230, 120, 170, 1.0f, "RASG-1" }, // 1st building
        { 180, 230,  90, 140, 1.0f, "YKSG-1" }, // 2nd building
        { 290, 230, 140, 190, 1.0f, "AB4"    }, // 3rd building
        { 460, 230, 110, 160, 1.0f, "RASG-2" }, // 4th building
        { 590, 230, 160, 210, 1.0f, "AB1"    }  // 5th building
    };

    // This loop iterates through the building data array one by one
    for (auto &b : bs) {
        glPushMatrix(); // Save the current matrix
        glTranslatef(b.x, b.y, 0); // Translate the building to its specific 'x' and 'y' position
        glScalef(b.s, b.s, 1.0f);

        // --- Step 1: Draw the main body of the building ---
        setColor(0.78f, 0.80f, 0.86f); // Light gray-blue color
        rectFilled(0, 0, b.w, b.h); // Fill the building structure

        setColor(0.30f, 0.35f, 0.45f); // Dark gray border color
        rectOutlineDDA(0, 0, (int)b.w, (int)b.h); // Draw building outline using DDA

        // --- Step 2: Draw the building windows ---
        int cols = 4, rows = 5; // 4 columns and 5 rows of windows
        float wx = b.w / (cols + 1), wy = b.h / (rows + 1); // Calculate window spacing
        for (int r = 1; r <= rows; r++) {
            for (int c = 1; c <= cols; c++) {
                setColor(0.55f, 0.70f, 0.90f); // Blue glass color for windows
                rectFilled(c * wx - 10, r * wy - 8, 18, 14); // Draw each window
            }
        }

        // --- Step 3: Draw the signboard above the building ---
        float sw = 95.0f, sh = 24.0f; // Signboard width (95) and height (24)
        float sx = (b.w - sw) / 2.0f; // Calculate position to center the signboard horizontally
        float sy = b.h + 6.0f; // Calculate position slightly above (6 pixels) the building roof

        setColor(0.20f, 0.40f, 0.80f); // Blue background color for the signboard
        rectFilled(sx, sy, sw, sh); // Draw the signboard box

        setColor(0.10f, 0.20f, 0.50f); // Dark blue outline color for the signboard
        rectOutline(sx, sy, sw, sh); // Draw the signboard border

        // --- Step 4: Render the building name text inside the signboard ---
        setColor(1.0f, 1.0f, 1.0f); // White color for the text
        // Adding 'sx + 8.0f' padding from the left to center the text
        renderText(sx + 8.0f, sy + 6.0f, b.name); 

        glPopMatrix(); // Restore the previous matrix
    }
}

static void drawSunMoon() {
    // Code to draw the sun (creating a yellow-orange circle)
    setColor(1.0f, 0.85f, 0.20f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(880, 520);
    for (int i = 0; i <= 360; i += 10) {
        float angle = i * 3.1415926f / 180.0f;
        glVertex2f(880 + 35 * cos(angle), 520 + 35 * sin(angle));
    }
    glEnd();
}

static void drawFilledCircle(float cx, float cy, float r, int segments = 40) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        float angle = i * 2.0f * 3.1415926f / segments;
        glVertex2f(cx + r * std::cos(angle), cy + r * std::sin(angle));
    }
    glEnd();
}

static void drawCloud() {
    // Forming a cloud shape by placing three circles side by side
    setColor(1.0f, 1.0f, 1.0f);
    drawFilledCircle(-20,  2, 18);
    drawFilledCircle(  5, 10, 22);
    drawFilledCircle( 30,  2, 18);

    glPointSize(2.0f);
    glBegin(GL_POINTS);
    circleMidpoint(-20,  2, 18);
    circleMidpoint(  5, 10, 22);
    circleMidpoint( 30,  2, 18);
    glEnd();
}

static void drawStation() {
    // Draw the metro station platform and back wall
    setColor(0.60f, 0.60f, 0.62f);
    rectFilled(0, 150, W, 80); // Platform body

    setColor(0.95f, 0.90f, 0.20f);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    lineBresenham(0, 150, W, 150); // Yellow safety line on the platform
    glEnd();

    setColor(0.88f, 0.88f, 0.90f);
    rectFilled(680, 230, 280, 170); // Back wall of the station

    setColor(0.25f, 0.30f, 0.40f);
    glPointSize(2.0f);
    rectOutlineBresenham(680, 230, 280, 170);

    // Main signboard for the station ("DIU METRO STATION")
    setColor(0.20f, 0.40f, 0.80f);
    rectFilled(690, 350, 260, 40);

    setColor(1.0f, 1.0f, 1.0f);
    renderText(710, 362, "DIU METRO STATION");
}

static void drawTrack() {
    // Draw the railway tracks and sleepers
    setColor(0.25f, 0.25f, 0.25f);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    lineBresenham(0, 120, W, 120); // Back track line
    lineBresenham(0,  95, W,  95); // Front track line
    glEnd();

    setColor(0.45f, 0.30f, 0.20f);
    for (int x = 0; x < W; x += 35)
        rectFilled((float)x, 92.0f, 18.0f, 32.0f); // Wooden sleepers
}

static void drawSignal(bool green) {
    // Draw the traffic signal pole and box
    setColor(0.20f, 0.20f, 0.22f);
    rectFilled(610, 150, 12, 140); // Pole
    
    setColor(0.12f, 0.12f, 0.14f);
    rectFilled(590, 260, 55, 85); // Signal box

    // Signal lights (Red and Green)
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    if (!green) {
        setColor(1.0f, 0.15f, 0.15f); circleMidpoint(617, 320, 12); // Red ON
        setColor(0.10f, 0.35f, 0.10f); circleMidpoint(617, 285, 12); // Green OFF
    } else {
        setColor(0.35f, 0.10f, 0.10f); circleMidpoint(617, 320, 12); // Red OFF
        setColor(0.15f, 1.0f, 0.20f); circleMidpoint(617, 285, 12); // Green ON
    }
    glEnd();
}

// --------------------------- Train + Passengers ---------------------------
enum TrainState {
    TS_MOVING_TO_STATION, TS_ARRIVING, TS_STOPPED_SIGNAL_RED,
    TS_DOORS_OPENING, TS_PASSENGERS_BOARDING, TS_DOORS_CLOSING,
    TS_SIGNAL_GREEN_WAIT, TS_MOVING_AWAY
};

static TrainState gState = TS_MOVING_TO_STATION;
static float gTrainX = -520.0f;
static const float TRAIN_Y = 89.0f; 

static float gTrainSpeed = 220.0f;
static float gDoorOpen = 0.0f;
static bool  gSignalGreen = true;

static const float STATION_STOP_X = 420.0f;
static const float TRAIN_LENGTH = 520.0f;

struct Passenger {
    bool active; float x, y, speed;
};

static Passenger p1, p2;

static void spawnPassengers() {
    // Positions of the waiting passengers at the station
    p1 = {true, 760.0f, 170.0f, 90.0f};
    p2 = {true, 820.0f, 170.0f, 80.0f};
}

static void drawPassenger(const Passenger& p) {
    if (!p.active) return;
    glPushMatrix();
    glTranslatef(p.x, p.y, 0);

    setColor(0.20f, 0.35f, 0.85f);
    rectFilled(-6, 0, 12, 26); // Passenger's body

    glPointSize(2.0f);
    glBegin(GL_POINTS);
    setColor(1.0f, 0.85f, 0.70f);
    circleMidpoint(0, 34, 8);  // Passenger's head
    glEnd();

    setColor(0.10f, 0.10f, 0.12f);
    rectFilled(-5, -14, 4, 14); // Left leg
    rectFilled(1, -14, 4, 14);  // Right leg
    glPopMatrix();
}

static void drawWheel(float cx, float cy, float r) {
    // Function to draw train wheels (solid black circle)
    glPushMatrix();
    glTranslatef(cx, cy, 0);
    setColor(0.05f, 0.05f, 0.05f); 
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (int i = 0; i <= 30; i++) {
        float angle = i * 2.0f * 3.1415926f / 30;
        glVertex2f(r * std::cos(angle), r * std::sin(angle));
    }
    glEnd();
    glPopMatrix();
}

// ---- Pseudo-3D helpers ----
static const float DEPTH3D_X = 15.0f;
static const float DEPTH3D_Y = 12.0f;

// Functions to draw the top roof and side faces for a 3D train look
static void drawTopFace3D(float x, float y, float w, float h) {
    float topY = y + h;
    glBegin(GL_QUADS);
    glVertex2f(x, topY); glVertex2f(x + w, topY);
    glVertex2f(x + w + DEPTH3D_X, topY + DEPTH3D_Y);
    glVertex2f(x + DEPTH3D_X, topY + DEPTH3D_Y);
    glEnd();
}

static void drawRightFace3D(float x, float y, float w, float h) {
    float rightX = x + w;
    glBegin(GL_QUADS);
    glVertex2f(rightX, y); glVertex2f(rightX + DEPTH3D_X, y + DEPTH3D_Y);
    glVertex2f(rightX + DEPTH3D_X, y + h + DEPTH3D_Y); glVertex2f(rightX, y + h);
    glEnd();
}

static void draw3DEdges(float x, float y, float w, float h) {
    int ix = (int)x, iy = (int)y, iw = (int)w, ih = (int)h;
    int dx = (int)DEPTH3D_X, dy = (int)DEPTH3D_Y;

    glBegin(GL_POINTS);
    lineBresenham(ix, iy + ih, ix + dx, iy + ih + dy);
    lineBresenham(ix + dx, iy + ih + dy, ix + iw + dx, iy + ih + dy);
    lineBresenham(ix + iw, iy + ih, ix + iw + dx, iy + ih + dy);
    lineBresenham(ix + iw + dx, iy + dy, ix + iw + dx, iy + ih + dy);
    lineBresenham(ix + iw, iy, ix + iw + dx, iy + dy);
    glEnd();
}

static void drawTrain() {
    // Code to draw the entire train (coaches, doors, and engine)
    glPushMatrix();
    glTranslatef(gTrainX, TRAIN_Y, 0); 

    const int coaches = 3; // 3 coaches
    const float coachW = 170.0f, coachH = 70.0f, gap = 8.0f;

    for (int i = 0; i < coaches; i++) {
        float ox = i * (coachW + gap);

        setColor(0.60f, 0.14f, 0.14f); drawRightFace3D(ox, 20, coachW, coachH);
        setColor(0.38f, 0.52f, 0.68f);
        glBegin(GL_QUADS);
        glVertex2f(ox + coachW, 55); glVertex2f(ox + coachW + DEPTH3D_X, 55 + DEPTH3D_Y);
        glVertex2f(ox + coachW + DEPTH3D_X, 77 + DEPTH3D_Y); glVertex2f(ox + coachW, 77);
        glEnd();
        
        setColor(0.52f, 0.10f, 0.10f); drawRightFace3D(ox, 85, coachW, 12);
        setColor(0.90f, 0.20f, 0.18f); drawTopFace3D(ox, 85, coachW, 12);
        
        setColor(0.20f, 0.20f, 0.22f);
        glPointSize(2.0f); draw3DEdges(ox, 20, coachW, 77);

        setColor(0.92f, 0.22f, 0.22f); rectFilled(ox, 20, coachW, coachH); // Coach body (Red)
        setColor(0.80f, 0.15f, 0.15f); rectFilled(ox, 85, coachW, 12);     
        setColor(0.55f, 0.75f, 0.95f); rectFilled(ox + 15, 55, coachW - 30, 22); // Coach windows
        
        setColor(0.20f, 0.20f, 0.22f);
        glPointSize(2.0f); rectOutlineBresenham((int)ox, 20, (int)coachW, (int)coachH + 12);

        if (i == 1) { // Draw a door only in the middle coach
            float doorX = ox + 65, doorY = 22, doorW = 40, doorH = 65;
            bool gatesActive = (gState == TS_DOORS_OPENING || gState == TS_PASSENGERS_BOARDING || gState == TS_DOORS_CLOSING);

            if (gatesActive) setColor(1.0f, 1.0f, 1.0f); // White color when the door is open
            else setColor(0.92f, 0.22f, 0.22f); // Red color when the door is closed
            rectFilled(doorX, doorY, doorW, doorH); 

            // Animation for sliding the door to the right
            float slide = doorW * gDoorOpen;
            if (doorW - slide > 0.0f) {
                rectFilled(doorX + slide, doorY, doorW - slide, doorH); 
            }

            if (gatesActive) setColor(1.0f, 1.0f, 1.0f);
            else setColor(0.80f, 0.15f, 0.15f);
            rectOutline(doorX, doorY, doorW, doorH); 
        }
    }

    // Draw the train's engine (Cab)
    float cabX = coaches * (coachW + gap);
    setColor(0.55f, 0.13f, 0.13f); drawRightFace3D(cabX, 30, 70, 60);
    setColor(0.38f, 0.52f, 0.68f);
    glBegin(GL_QUADS);
    glVertex2f(cabX + 70, 60); glVertex2f(cabX + 70 + DEPTH3D_X, 60 + DEPTH3D_Y);
    glVertex2f(cabX + 70 + DEPTH3D_X, 78 + DEPTH3D_Y); glVertex2f(cabX + 70, 78);
    glEnd();
    
    setColor(0.95f, 0.23f, 0.23f); drawTopFace3D(cabX, 30, 70, 60);
    setColor(0.20f, 0.20f, 0.22f); glPointSize(2.0f); draw3DEdges(cabX, 30, 70, 60);

    setColor(0.85f, 0.20f, 0.20f); rectFilled(cabX, 30, 70, 60);
    setColor(0.55f, 0.75f, 0.95f); rectFilled(cabX + 20, 60, 35, 18); 

    // Place wheels underneath the coaches and engine
    for (int i = 0; i < coaches; i++) {
        float ox = i * (coachW + gap);
        drawWheel(ox + 35, 18, 12);
        drawWheel(ox + coachW - 35, 18, 12);
    }
    drawWheel(cabX + 20, 18, 12); drawWheel(cabX + 55, 18, 12);
    glPopMatrix();
}

// --------------------------- State Machine ---------------------------
static float c1x = 120.0f, c2x = 520.0f, c3x = 860.0f;
static float cloudSpeed = 25.0f;
static float stateTimer = 0.0f;

static void updateStateMachine(float dt) {
    // This function controls the entire simulation logic (train arrival, signal changes, door opening, etc.)
    stateTimer += dt;

    switch (gState) {
        case TS_MOVING_TO_STATION: // Train is moving towards the station
            gSignalGreen = true; gDoorOpen = 0.0f;
            gTrainX += gTrainSpeed * dt;
            if (gTrainX >= STATION_STOP_X) {
                gTrainX = STATION_STOP_X; gState = TS_ARRIVING; stateTimer = 0.0f;
            }
            break;
        case TS_ARRIVING: // Train is arriving at the station
            gSignalGreen = true;
            if (stateTimer > 0.35f) { gState = TS_STOPPED_SIGNAL_RED; stateTimer = 0.0f; }
            break;
        case TS_STOPPED_SIGNAL_RED: // Signal turns red, train stops
            gSignalGreen = false;
            if (stateTimer > 0.6f) { gState = TS_DOORS_OPENING; stateTimer = 0.0f; }
            break;
        case TS_DOORS_OPENING: // Doors are opening
            gSignalGreen = false;
            gDoorOpen = std::min(1.0f, gDoorOpen + 1.3f * dt);
            if (gDoorOpen >= 1.0f && stateTimer > 0.2f) { gState = TS_PASSENGERS_BOARDING; stateTimer = 0.0f; }
            break;
        case TS_PASSENGERS_BOARDING: // Passengers are boarding the train
        {
            gSignalGreen = false;
            float doorX = gTrainX + 240.0f + 65.0f;
            auto movePassenger = [&](Passenger& p) {
                if (!p.active) return;
                float dx = doorX - p.x;
                float step = p.speed * dt;
                p.x = (std::fabs(dx) <= step) ? doorX : p.x + (dx > 0 ? step : -step);
                if (std::fabs(p.x - doorX) < 2.0f && gDoorOpen > 0.95f) p.active = false;
            };

            movePassenger(p1); movePassenger(p2);
            if (!p1.active && !p2.active && stateTimer > 0.4f) {
                gState = TS_DOORS_CLOSING; stateTimer = 0.0f;
            }
            break;
        }
        case TS_DOORS_CLOSING: // Doors are closing
            gSignalGreen = false;
            gDoorOpen = std::max(0.0f, gDoorOpen - 1.3f * dt);
            if (gDoorOpen <= 0.0f) { gState = TS_SIGNAL_GREEN_WAIT; stateTimer = 0.0f; }
            break;
        case TS_SIGNAL_GREEN_WAIT: // Waiting for the green signal
            gSignalGreen = true;
            if (stateTimer > 0.5f) { gState = TS_MOVING_AWAY; stateTimer = 0.0f; }
            break;
        case TS_MOVING_AWAY: // Train is moving away from the station
            gSignalGreen = true;
            gTrainX += gTrainSpeed * dt;
            if (gTrainX > W + 50.0f) {
                gTrainX = -TRAIN_LENGTH; gDoorOpen = 0.0f;
                spawnPassengers();
                gState = TS_MOVING_TO_STATION; stateTimer = 0.0f;
            }
            break;
    }
}

// --------------------------- Display & Core ---------------------------
static void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    setColor(0.55f, 0.80f, 0.98f); rectFilled(0, 0, W, H); // Sky color
    setColor(0.45f, 0.75f, 0.45f); rectFilled(0, 0, W, 150); // Ground color

    // Call all functions to render them on the screen one by one
    drawSunMoon(); 
    drawBuildings(); 
    drawStation();
    drawTrack(); 
    drawSignal(gSignalGreen);

    // Setup for clouds floating in the sky
    glPushMatrix(); glTranslatef(c1x, 520.0f, 0); drawCloud(); glPopMatrix();
    glPushMatrix(); glTranslatef(c2x, 480.0f, 0); glScalef(1.1f, 1.1f, 1.0f); drawCloud(); glPopMatrix();
    glPushMatrix(); glTranslatef(c3x, 540.0f, 0); glScalef(0.9f, 0.9f, 1.0f); drawCloud(); glPopMatrix();

    drawPassenger(p1); drawPassenger(p2);
    drawTrain();

    glutSwapBuffers();
}

static void timer(int) {
    // Update cloud positions
    c1x += cloudSpeed * DT; c2x += (cloudSpeed * 0.8f) * DT; c3x += (cloudSpeed * 1.1f) * DT;
    if (c1x > W + 60) c1x = -60;
    if (c2x > W + 60) c2x = -60;
    if (c3x > W + 60) c3x = -60;

    updateStateMachine(DT);
    glutPostRedisplay(); // Send request to render a new frame
    glutTimerFunc(TIMER_MS, timer, 0);
}

static void keyboard(unsigned char key, int, int) {
    if (key == 27) exit(0); // Exit program when ESC is pressed
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(W, H); // Set window size
    glutInitWindowPosition(80, 60);
    glutCreateWindow("Metro Rail Simulation - DIU Campus"); // Window title

    glClearColor(0, 0, 0, 1);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluOrtho2D(0, W, 0, H);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glDisable(GL_DEPTH_TEST); glPointSize(2.0f);

    spawnPassengers(); // Spawn passengers at startup

    // Set default GLUT callback functions
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(TIMER_MS, timer, 0);
    glutMainLoop();
    return 0;
}