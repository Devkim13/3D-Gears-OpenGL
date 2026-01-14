/*
    Projet - Roues dentées 3D (même taille pour les deux premières, plus grande pour la troisième)
    Modifications :
      - Épaisseur et trou plus grands pour la roue verte
      - Troisième roue bleue (rayon x2, dents x2, trou différent, épaisseur plus fine)
      - Placement en dessous de la roue rouge, adaptation de la rotation
*/

#include <iostream>
#include <cmath>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ------------------------------------------------------------------
// Variables globales pour la gestion des touches et des rotations
// ------------------------------------------------------------------
static bool animate      = false; // Touche A (Q pour clavie AZERTY) : animation continue
static bool spacePressed = false; // Touche Espace : rotation pas à pas
static bool flag_fill    = false; // Touche F : bascule entre wireframe et remplissage (faces avant/arrière)

// Angles de rotation pour les trois roues
static double rotationRouge = 0.0;
static double rotationVerte = 0.0;
static double rotationBleue = 0.0;

// ------------------------------------------------------------------
// Callback clavier : gère A (Q), Espace, F, Echap
// ------------------------------------------------------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Relâchement de la barre Espace
    if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
        spacePressed = false;
        return;
    }
    // Pression ou répétition
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_A) {
            // Bascule de l'animation continue
            animate = !animate;
            std::cout << "[Info] Touche A : animate = " << animate << std::endl;
        }
        else if (key == GLFW_KEY_SPACE) {
            // Active la rotation pas à pas
            spacePressed = true;
            std::cout << "[Info] Touche Espace : rotation pas à pas" << std::endl;
        }
        else if (key == GLFW_KEY_F) {
            // Bascule du remplissage (faces avant/arrière)
            flag_fill = !flag_fill;
            std::cout << "[Info] Touche F : flag_fill = " << (flag_fill ? "true" : "false") << std::endl;
        }
        else if (key == GLFW_KEY_ESCAPE) {
            // Quitter
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

// ------------------------------------------------------------------
// Fonction pour dessiner les facettes d'un bloc de dent
// (A..H sur face avant, A'..H' sur face arrière)
// ------------------------------------------------------------------
void dessiner_facettes_bloc(
    double Ax, double Ay,
    double Bx, double By,
    double Cx, double Cy,
    double Dx, double Dy,
    double Ex, double Ey,
    double Fx, double Fy,
    double Gx, double Gy,
    double Hx, double Hy,
    double ep_roue,
    float coul_r, float coul_v, float coul_b
)
{
    double z1 = +ep_roue / 2.0;  
    double z2 = -ep_roue / 2.0; 

    // Couleurs (faces perpendiculaires vs obliques)
    float r_perp = coul_r * 0.8f;
    float v_perp = coul_v * 0.8f;
    float b_perp = coul_b * 0.8f;

    float r_obl = coul_r * 0.6f;
    float v_obl = coul_v * 0.6f;
    float b_obl = coul_b * 0.6f;

    // 1) A H H' A'
    glColor3f(r_perp, v_perp, b_perp);
    glBegin(GL_QUADS);
        glVertex3d(Ax, Ay, z1);
        glVertex3d(Hx, Hy, z1);
        glVertex3d(Hx, Hy, z2);
        glVertex3d(Ax, Ay, z2);
    glEnd();

    // 2) H G G' H'
    glColor3f(r_obl, v_obl, b_obl);
    glBegin(GL_QUADS);
        glVertex3d(Hx, Hy, z1);
        glVertex3d(Gx, Gy, z1);
        glVertex3d(Gx, Gy, z2);
        glVertex3d(Hx, Hy, z2);
    glEnd();

    // 3) B C C' B'
    glColor3f(r_perp, v_perp, b_perp);
    glBegin(GL_QUADS);
        glVertex3d(Bx, By, z1);
        glVertex3d(Cx, Cy, z1);
        glVertex3d(Cx, Cy, z2);
        glVertex3d(Bx, By, z2);
    glEnd();

    // 4) C D D' C'
    glColor3f(r_obl, v_obl, b_obl);
    glBegin(GL_QUADS);
        glVertex3d(Cx, Cy, z1);
        glVertex3d(Dx, Dy, z1);
        glVertex3d(Dx, Dy, z2);
        glVertex3d(Cx, Cy, z2);
    glEnd();

    // 5) D E E' D'
    glColor3f(r_perp, v_perp, b_perp);
    glBegin(GL_QUADS);
        glVertex3d(Dx, Dy, z1);
        glVertex3d(Ex, Ey, z1);
        glVertex3d(Ex, Ey, z2);
        glVertex3d(Dx, Dy, z2);
    glEnd();

    // 6) E F F' E'
    glColor3f(r_obl, v_obl, b_obl);
    glBegin(GL_QUADS);
        glVertex3d(Ex, Ey, z1);
        glVertex3d(Fx, Fy, z1);
        glVertex3d(Fx, Fy, z2);
        glVertex3d(Ex, Ey, z2);
    glEnd();
}

// ------------------------------------------------------------------
// Classe Roue : définition et méthodes pour dessiner une roue dentée 3D
// ------------------------------------------------------------------
class Roue {
public:
    int m_nb_dents;       // Nombre de dents
    double m_r_trou;      // Rayon du trou central
    double m_r_roue;      // Rayon moyen
    double m_h_dent;      // Hauteur des dents
    float m_coul_r, m_coul_v, m_coul_b;  // Couleur (RGB)
    double m_ep_roue;     // Épaisseur de la roue

    Roue(int nb_dents, double r_trou, double r_roue, double h_dent,
         float coul_r, float coul_v, float coul_b, double ep_roue)
    : m_nb_dents(nb_dents), m_r_trou(r_trou), m_r_roue(r_roue), m_h_dent(h_dent),
      m_coul_r(coul_r), m_coul_v(coul_v), m_coul_b(coul_b), m_ep_roue(ep_roue)
    { }

    // Dessine un bloc de dent (A,B,C,D,E,F,G,H) en 2D (face avant ou arrière)
    void dessiner_bloc_dent_2D() const {
        double alpha_deg = 360.0 / m_nb_dents;
        double alpha_rad = alpha_deg * M_PI / 180.0;

        double angleA = 0.0;
        double angleB = 0.0;
        double angleC = (alpha_deg / 4.0) * M_PI / 180.0;
        double angleD = (alpha_deg / 2.0) * M_PI / 180.0;
        double angleE = (3.0 * alpha_deg / 4.0) * M_PI / 180.0;
        double angleF = alpha_rad;
        double angleG = angleF;
        double angleH = (alpha_deg / 2.0) * M_PI / 180.0;

        double Ax = m_r_trou * cos(angleA);
        double Ay = m_r_trou * sin(angleA);
        double Bx = (m_r_roue - m_h_dent/2.0) * cos(angleB);
        double By = (m_r_roue - m_h_dent/2.0) * sin(angleB);
        double Cx = (m_r_roue - m_h_dent/2.0) * cos(angleC);
        double Cy = (m_r_roue - m_h_dent/2.0) * sin(angleC);
        double Dx = (m_r_roue + m_h_dent/2.0) * cos(angleD);
        double Dy = (m_r_roue + m_h_dent/2.0) * sin(angleD);
        double Ex = (m_r_roue + m_h_dent/2.0) * cos(angleE);
        double Ey = (m_r_roue + m_h_dent/2.0) * sin(angleE);
        double Fx = (m_r_roue - m_h_dent/2.0) * cos(angleF);
        double Fy = (m_r_roue - m_h_dent/2.0) * sin(angleF);
        double Gx = m_r_trou * cos(angleG);
        double Gy = m_r_trou * sin(angleG);
        double Hx = m_r_trou * cos(angleH);
        double Hy = m_r_trou * sin(angleH);

        // Dessin en wireframe ou polygon, selon flag_fill
        if (!flag_fill) glBegin(GL_LINE_LOOP);
        else            glBegin(GL_POLYGON);
            glVertex2d(Ax, Ay);
            glVertex2d(Bx, By);
            glVertex2d(Cx, Cy);
            glVertex2d(Dx, Dy);
            glVertex2d(Ex, Ey);
            glVertex2d(Fx, Fy);
            glVertex2d(Gx, Gy);
            glVertex2d(Hx, Hy);
        glEnd();
    }

    // Dessin de la face (avant ou arrière) en répétant le bloc pour chaque dent
    void dessiner_cote_roue_2D() const {
        double alpha_deg = 360.0 / m_nb_dents;
        for (int i = 0; i < m_nb_dents; i++) {
            glPushMatrix();
                glRotatef(i * alpha_deg, 0.0f, 0.0f, 1.0f);
                dessiner_bloc_dent_2D();
            glPopMatrix();
        }
    }

    // Dessine les facettes pour toutes les dents, en faisant la rotation locale
    void dessiner_facettes_cote_roue() const {
        double alpha_deg = 360.0 / m_nb_dents;
        for (int i = 0; i < m_nb_dents; i++) {
            glPushMatrix();
                glRotatef(i * alpha_deg, 0.0f, 0.0f, 1.0f);

                // Calcul des 8 sommets A..H en 2D
                double angleA = 0.0;
                double angleB = 0.0;
                double angleC = (alpha_deg / 4.0) * M_PI / 180.0;
                double angleD = (alpha_deg / 2.0) * M_PI / 180.0;
                double angleE = (3.0 * alpha_deg / 4.0) * M_PI / 180.0;
                double angleF = alpha_deg * M_PI / 180.0;
                double angleG = angleF;
                double angleH = (alpha_deg / 2.0) * M_PI / 180.0;

                double Ax = m_r_trou * cos(angleA);
                double Ay = m_r_trou * sin(angleA);
                double Bx = (m_r_roue - m_h_dent/2.0) * cos(angleB);
                double By = (m_r_roue - m_h_dent/2.0) * sin(angleB);
                double Cx = (m_r_roue - m_h_dent/2.0) * cos(angleC);
                double Cy = (m_r_roue - m_h_dent/2.0) * sin(angleC);
                double Dx = (m_r_roue + m_h_dent/2.0) * cos(angleD);
                double Dy = (m_r_roue + m_h_dent/2.0) * sin(angleD);
                double Ex = (m_r_roue + m_h_dent/2.0) * cos(angleE);
                double Ey = (m_r_roue + m_h_dent/2.0) * sin(angleE);
                double Fx = (m_r_roue - m_h_dent/2.0) * cos(angleF);
                double Fy = (m_r_roue - m_h_dent/2.0) * sin(angleF);
                double Gx = m_r_trou * cos(angleG);
                double Gy = m_r_trou * sin(angleG);
                double Hx = m_r_trou * cos(angleH);
                double Hy = m_r_trou * sin(angleH);

                // Appel à dessiner_facettes_bloc
                dessiner_facettes_bloc(
                    Ax, Ay, Bx, By, Cx, Cy, Dx, Dy,
                    Ex, Ey, Fx, Fy, Gx, Gy, Hx, Hy,
                    m_ep_roue,
                    m_coul_r, m_coul_v, m_coul_b
                );
            glPopMatrix();
        }
    }

    // Méthode principale de dessin 3D
    void dessiner_roue() const {
        glColor3f(m_coul_r, m_coul_v, m_coul_b);

        // Face avant
        glPushMatrix();
            glTranslatef(0.0f, 0.0f, m_ep_roue / 2.0f);
            dessiner_cote_roue_2D();
        glPopMatrix();

        // Face arrière
        glPushMatrix();
            glTranslatef(0.0f, 0.0f, -m_ep_roue / 2.0f);
            dessiner_cote_roue_2D();
        glPopMatrix();

        // Facettes
        dessiner_facettes_cote_roue();
    }
};

// ------------------------------------------------------------------
// Création des trois roues :
// 1) roueRouge
// 2) roueVerte (épaisseur plus grande, trou plus grand)
// 3) roueBleue (x2 rayon, x2 dents, trou différent, épaisseur plus fine)
// ------------------------------------------------------------------

// Roue rouge (identique)
Roue roueRouge(10, 0.2, 0.5, 0.2, 1.0f, 0.0f, 0.0f, 0.05);

// Roue verte : on augmente son trou (ex. 0.3) et son épaisseur (ex. 0.08)
Roue roueVerte(10, 0.3, 0.5, 0.2, 0.0f, 1.0f, 0.0f, 0.08);

// Roue bleue : rayon x2 => 1.0, nb_dents x2 => 20, trou plus grand (ex. 0.4), épaisseur plus fine (ex. 0.03)
Roue roueBleue(20, 0.4, 1.0, 0.2, 0.0f, 0.0f, 1.0f, 0.03);

// ------------------------------------------------------------------
// Positions et décalages
// 1) rouge et verte, côte à côte
// 2) bleue en dessous de la rouge
// ------------------------------------------------------------------

// Pour rouge et verte, on garde l'espacement horizontal
static double distance_centres = 0.82;
static double xRouge = -distance_centres / 2.0;
static double xVerte =  distance_centres / 2.0;
static double yCentre = 0.0;

// Roue bleue : on la place plus bas (ex. y = -1.6)
static double xBleue = -0.66;
static double yBleue = -1.54;

// Petit décalage angulaire pour l'emboîtement rouge/verte
static double decalageAngulaire = 360.0 / (4.0 * 10); // 9° pour 10 dents

// On peut choisir un ratio pour la bleue (si on veut la synchroniser avec la rouge)
// Ex. si rouge = 10 dents, bleue = 20 dents => ratio 1:2
// => rotationBleue = - (10/20) * rotationRouge = -0.5 * rotationRouge
// On l'appliquera dans update()

// ------------------------------------------------------------------
// update() : applique la rotation selon animate et spacePressed
// ------------------------------------------------------------------
void update() {
    if (animate) {
        rotationRouge += 0.5;
        if (rotationRouge >= 360.0) rotationRouge -= 360.0;
        // Roue verte : sens inverse, ratio 1:1 (même nb de dents que la rouge)
        rotationVerte = -rotationRouge;
        // Roue bleue : ratio 10:20 => 1:2 => tourne deux fois moins vite, sens opposé
        rotationBleue = -( (double)roueRouge.m_nb_dents / (double)roueBleue.m_nb_dents ) * rotationRouge; 
    }
    if (spacePressed) {
        rotationRouge += 0.5;
        if (rotationRouge >= 360.0) rotationRouge -= 360.0;
        rotationVerte = -rotationRouge;
        rotationBleue = -( (double)roueRouge.m_nb_dents / (double)roueBleue.m_nb_dents ) * rotationRouge; 
    }
}

// ------------------------------------------------------------------
// display() : dessine la scène
// ------------------------------------------------------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, 1.0f, 0.0f);
    // Dessin de la roue rouge
    glPushMatrix();
        glTranslatef(xRouge, yCentre, 0.0f);
        glRotatef(rotationRouge, 0.0f, 0.0f, 1.0f);
        roueRouge.dessiner_roue();
    glPopMatrix();

    // Dessin de la roue verte (avec décalageAngulaire pour l'emboîtement)
    glPushMatrix();
        glTranslatef(0.64, yCentre, 0.0f);
        glRotatef(rotationVerte + decalageAngulaire, 0.0f, 0.0f, 1.0f);
        roueVerte.dessiner_roue();
    glPopMatrix();

    // Dessin de la roue bleue (en dessous)
    glPushMatrix();
        glTranslatef(xBleue, yBleue, 0.0f);
        glRotatef(rotationBleue, 0.0f, 0.0f, 1.0f);
        roueBleue.dessiner_roue();
    glPopMatrix();

    glfwSwapBuffers(glfwGetCurrentContext());
    glFlush();
}

// ------------------------------------------------------------------
// main() : initialisation GLFW, boucle principale
// ------------------------------------------------------------------
int main() {
    if (!glfwInit()) {
        std::cerr << "Erreur d'initialisation de GLFW" << std::endl;
        return -1;
    }
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "3 Roues Dentées 3D - A(Q)/Espace/F", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glEnable(GL_DEPTH_TEST);

    // Enregistrement du callback clavier
    glfwSetKeyCallback(window, key_callback);

    // Configuration de la projection orthographique
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double ratio = 800.0 / 600.0;
    glOrtho(-2.0 * ratio, 2.0 * ratio, -2.0, 2.0, -10.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    

        
    // Boucle principale
    while (!glfwWindowShouldClose(window)) {
        update();
        display();
        glfwPollEvents();
    }
 
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

