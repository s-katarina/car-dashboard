// Autor: Bogdan Davinic

#define _CRT_SECURE_NO_WARNINGS
#define CRES 30 // Circle Resolution = Rezolucija kruga

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>   
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Biblioteke za pokretanje generatora podataka
#include "TestBed.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned int compileShader(GLenum type, const char* source); 
unsigned int createShader(const char* vsSource, const char* fsSource);
static unsigned loadImageToTexture(const char* filePath);


int main(void)
{

    //Inicijalizacija objekta tipa Car
    Car car = getCar();
    
    if (!glfwInit()) 
    {
        std::cout<<"GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window; 
    unsigned int wWidth = 1200;
    unsigned int wHeight = 600;
    const char wTitle[] = "[Generic Title]";
    window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL); 
    if (window == NULL) 
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate(); 
        return 2; 
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) 
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }

    unsigned int VAO[10];
    glGenVertexArrays(10, VAO);

    unsigned int VBO[10];
    glGenBuffers(10, VBO);


    // --------------- TEKSTURA BRZINOMETRA  --------------- 

    unsigned int textureShader = createShader("texture.vert", "texture.frag");

    unsigned meterTexture = loadImageToTexture("res/meter1.jpg");
    glBindTexture(GL_TEXTURE_2D, meterTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//S = U = X    GL_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);// T = V = Y
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   //GL_NEAREST, GL_LINEAR
    glBindTexture(GL_TEXTURE_2D, 0);
    unsigned uTexLoc = glGetUniformLocation(textureShader, "uTex");
    glUniform1i(uTexLoc, 0); // Indeks teksturne jedinice (sa koje teksture ce se citati boje)

    float meterVertices[] = {
        -1.0, 0.0,          0.0, 1.0,
        -0.2, 0.0,	        1.0, 1.0,
        -1.0,-1.0,	        0.0, 0.0,
        -0.2,-1.0,	        1.0, 0.0,
    };

    glBindVertexArray(VAO[0]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(meterVertices), meterVertices, GL_STATIC_DRAW);

    // na lokaciju 0 (za ulaz u vertex shader) idu koordinate
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    // na lokaciju 1 idu koordinate teksture pridruzene temenu 
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // --------------- KAZALJKA  --------------- 

    // boje red i blue
    float red = 0.0;
    float blue = 1.0;

    unsigned int stripShader = createShader("needle.vert", "needle.frag");
    unsigned int uR = glGetUniformLocation(stripShader, "uR");
    unsigned int uB = glGetUniformLocation(stripShader, "uB");
    GLint uniTrans = glGetUniformLocation(stripShader, "trans");

    float needleVertices[] = {
       -0.2, 0.0,
       0.0,  0.0,
       -0.2, 0.07,
       0.0,  0.07
    };

    glBindVertexArray(VAO[1]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(needleVertices), needleVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // --------------- LAMPICA ZA STEPEN PRENOSA  --------------- 

    float gearR = 1.0;
    float gearG = 0.0;
    float gearB = 0.0;

    unsigned int gearShader = createShader("gear.vert", "gear.frag");
    unsigned int uGearR = glGetUniformLocation(gearShader, "uGearR");
    unsigned int uGearG = glGetUniformLocation(gearShader, "uGearG");
    unsigned int uGearB = glGetUniformLocation(gearShader, "uGearB");

    float gearLampVertices[] = {
       -0.6, 0.6,
       -0.5,  0.6,
       -0.6, 0.5,
       -0.5,  0.5
    };

    glBindVertexArray(VAO[2]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gearLampVertices), gearLampVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // --------------- PROGRESS BAR ZA GORIVO  --------------- 

    unsigned int progressBarShader = createShader("progressBar.vert", "progressBar.frag");
    GLint uniGasTrans = glGetUniformLocation(progressBarShader, "trans");

    float gasProgressBarVertices[] = {
       -0.3, 0.2,       0.0, 1.0, 0.0,
       0.0,  0.2,       0.0, 1.0, 0.0,
       -0.3, 0.0,       0.0, 1.0, 0.0,
       0.0,  0.0,       0.0, 1.0, 0.0,

       -0.3, 0.2,       0.0, 0.0, 0.0,
       0.0,  0.2,       0.0, 0.0, 0.0,
       -0.3, 0.0,       0.0, 0.0, 0.0,
       0.0,  0.0,        0.0, 0.0, 0.0
    };

    glBindVertexArray(VAO[3]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gasProgressBarVertices), gasProgressBarVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // --------------- PROGRESS BAR ZA DODAVANJE BRZINE  --------------- 

    GLint uniSpeedTrans = glGetUniformLocation(progressBarShader, "trans");

    float speedProgressBarVertices[] = {
       -0.3, 0.2,       0.0, 0.0, 1.0,
       0.0,  0.2,       0.0, 0.0, 1.0,
       -0.3, 0.0,       0.0, 0.0, 1.0,
       0.0,  0.0,       0.0, 0.0, 1.0,

       -0.4, 0.2,       0.0, 0.0, 0.0,
       -0.1,  0.2,       0.0, 0.0, 0.0,
       -0.4, 0.0,       0.0, 0.0, 0.0,
       -0.1,  0.0,        0.0, 0.0, 0.0
    };

    glBindVertexArray(VAO[4]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(speedProgressBarVertices), speedProgressBarVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // --------------- DISPLEJ SA KRUGOM  --------------- 

    GLint uniDisplayTrans = glGetUniformLocation(progressBarShader, "trans");

    float displayVertices[] = {
       -0.3, 0.3,       0.0, 0.0, 0.0,
       0.3,  0.3,       0.0, 0.0, 0.0,
       -0.3, -0.3,       0.0, 0.0, 0.0,
       0.3,  -0.3,       0.0, 0.0, 0.0,
    };

    glBindVertexArray(VAO[5]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(displayVertices), displayVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ----- KRUG -----
    unsigned int circleShader = createShader("circle.vert", "circle.frag");

    GLint uniCircleTrans = glGetUniformLocation(circleShader, "trans");
    unsigned int uCirclePosX = glGetUniformLocation(circleShader, "uPosX");

    float circle[CRES * 2 + 4]; // +4 je za x i y koordinate centra kruga, i za x i y od nultog ugla
    float r = 0.1; 

    circle[0] = 0; //Centar X0
    circle[1] = 0; //Centar Y0
    int i;
    for (i = 0; i <= CRES; i++)
    {

        circle[2 + 2 * i] = r * cos((3.141592 / 180) * (i * 360 / CRES)); //Xi
        circle[2 + 2 * i + 1] = r * sin((3.141592 / 180) * (i * 360 / CRES)); //Yi
    }

    glBindVertexArray(VAO[6]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[6]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circle), circle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // --------------- TEKSTURA POKAZIVACA PRAVCA  --------------- 


    unsigned rightIndicatorTexture = loadImageToTexture("res/rightIndicator.png");
    glBindTexture(GL_TEXTURE_2D, rightIndicatorTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//S = U = X    GL_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);// T = V = Y
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   //GL_NEAREST, GL_LINEAR
    glBindTexture(GL_TEXTURE_2D, 0);

    float rightIndicatorVertices[] = {
         0.7, 0.9,          0.0, 1.0,
         0.9, 0.9,	        1.0, 1.0,
         0.7, 0.7,	        0.0, 0.0,
         0.9, 0.7,	        1.0, 0.0,
    };

    glBindVertexArray(VAO[7]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[7]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rightIndicatorVertices), rightIndicatorVertices, GL_STATIC_DRAW);

    // na lokaciju 0 (za ulaz u vertex shader) idu koordinate
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    // na lokaciju 1 idu koordinate teksture pridruzene temenu 
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    unsigned leftIndicatorTexture = loadImageToTexture("res/leftIndicator.png");
    glBindTexture(GL_TEXTURE_2D, leftIndicatorTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//S = U = X    GL_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);// T = V = Y
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   //GL_NEAREST, GL_LINEAR
    glBindTexture(GL_TEXTURE_2D, 0);

    float leftIndicatorVertices[] = {
         0.4, 0.9,          0.0, 1.0,
         0.6, 0.9,	        1.0, 1.0,
         0.4, 0.7,	        0.0, 0.0,
         0.6, 0.7,	        1.0, 0.0,
    };

    glBindVertexArray(VAO[8]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[8]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(leftIndicatorVertices), leftIndicatorVertices, GL_STATIC_DRAW);

    // na lokaciju 0 (za ulaz u vertex shader) idu koordinate
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    // na lokaciju 1 idu koordinate teksture pridruzene temenu 
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // --------------- TEKSTURA IMENA I PREZIMENA  --------------- 


    unsigned nameTexture = loadImageToTexture("res/ime.jpg");
    glBindTexture(GL_TEXTURE_2D, nameTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//S = U = X    GL_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);// T = V = Y
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   //GL_NEAREST, GL_LINEAR
    glBindTexture(GL_TEXTURE_2D, 0);

    float nameVertices[] = {
         0.6, -0.8,          0.0, 1.0,
         1.0, -0.8,	         1.0, 1.0,
         0.6, -1.0,	         0.0, 0.0,
         1.0, -1.0,	         1.0, 0.0,
    };

    glBindVertexArray(VAO[9]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[9]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(nameVertices), nameVertices, GL_STATIC_DRAW);

    // na lokaciju 0 (za ulaz u vertex shader) idu koordinate
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    // na lokaciju 1 idu koordinate teksture pridruzene temenu 
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    
    startSimulation(&car);

    float speedAmount = 0;

    float circlePositionX = 0;
    // krug se krece tako da je podesena granica displeja umanjena za poluprecnik kruga
    float circleBoundRight = 0.3 - r;
    float circleBoundLeft = -0.3 + r;
    float circleStep = 0.0001;

    bool rightIndicatorOn = false;
    bool leftIndicatorOn = false;    
    
    glClearColor(1, 1, 1, 1);
    
    while (!glfwWindowShouldClose(window)) 
    {
        glClear(GL_COLOR_BUFFER_BIT);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }


        // BRZINOMER 

        glUseProgram(textureShader);
        glBindVertexArray(VAO[0]);

        glActiveTexture(GL_TEXTURE0); //tekstura koja se bind-uje nakon ovoga ce se koristiti sa SAMPLER2D uniformom u sejderu koja odgovara njenom indeksu
        glBindTexture(GL_TEXTURE_2D, meterTexture);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

        glBindTexture(GL_TEXTURE_2D, 0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);


        // KAZALJKA

        glUseProgram(stripShader);
        glBindVertexArray(VAO[1]);

        glUniform1f(uR, red);
        glUniform1f(uB, blue);


        float speed = car.getSpeed();
        std::cout << "Speed: " << speed << "\n";

        // 180/100 znaci da brzinomer ima 100 podeoka rasporedjeni na polukrugu od 180 stepeni, ugao kazaljke je brzina * podeok 
        // oduzimanje "- (180.00 / 100.00) / 2" da se centar kazaljke postavi na podeok
        float angle = speed * (180.00 / 100.00) - (180.00 / 100.00) / 2;

        glm::mat4 trans = glm::mat4(1.0f);
        // skaliranje u odnosu na dimenzije ekrana 1200/600
        trans = glm::scale(trans, glm::vec3(1.0f, 1200.f/600.0f, 1.0f));
        // translacija na brzinomer
        trans = glm::translate(trans, glm::vec3(-0.6f, -0.38f, 0));
        trans = glm::rotate(
            trans,
            float(glm::radians(angle)),
            glm::vec3(0.0f, 0.0f, -1.0f)
        );
        trans = glm::scale(trans, glm::vec3(1.0, 0.2, 1.0f));

        glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // postavka boje kazaljke
        if (speed > 30) {
            red = 1.0;
            blue = 0.0;
        }
        else {
            red = 0.0;
            blue = 1.0;
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        // LAMPICA ZA STEPEN PRENOSA BRZINE

        glUseProgram(gearShader);
        glBindVertexArray(VAO[2]);

        glUniform1f(uGearR, gearR);
        glUniform1f(uGearG, gearG);
        glUniform1f(uGearB, gearB);


        int gear = car.getGear();
        std::cout << "Gear " << gear << "\n";

        if (gear == 1) {
            gearR = abs(sin(glfwGetTime())); gearG = 0.0; gearB = 0.0;
        }
        else if (gear == 2) {
            gearR = abs(sin(glfwGetTime())); gearG = abs(sin(glfwGetTime())); gearB = 0.0;
        }
        else if (gear == 3) {
            gearR = abs(sin(glfwGetTime())); gearG = 0.0; gearB = abs(sin(glfwGetTime()));
        }
        else if (gear == 4) {
            gearR = 0.0; gearG = 0.0; gearB = abs(sin(glfwGetTime()));
        }
        else if (gear == 5) {
            gearR = 0.0; gearG = abs(sin(glfwGetTime())); gearB = 0.0;
        }

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        // PROGRES BAR ZA GORIVO

        glUseProgram(progressBarShader);
        glBindVertexArray(VAO[3]);

        float gas = car.getFuelAmount();
        std::cout << "Gas: " << gas << "\n";


        glm::mat4 transGasMax = glm::mat4(1.0f);
        transGasMax = glm::translate(transGasMax, glm::vec3(-0.4f, 0.0f, 0));
        glUniformMatrix4fv(uniGasTrans, 1, GL_FALSE, glm::value_ptr(transGasMax));
        glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

        glm::mat4 transGas = glm::mat4(1.0f);
        transGas = glm::scale(transGas, glm::vec3(gas / car.getMaxFuelAmount(), 1.0f, 1.0f));
        transGas = glm::translate(transGas, glm::vec3(-0.4f, 0.0f, 0));
        glUniformMatrix4fv(uniGasTrans, 1, GL_FALSE, glm::value_ptr(transGas));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);


        glfwPollEvents();


        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        {
            speedAmount += 0.0005;
            if (speedAmount > 1.0) 
                speedAmount = 1.0;
        }
        else {
            speedAmount -= 0.0005;
            if (speedAmount < 0)
                speedAmount = 0;
        }

        // PROGRES BAR ZA DODAVANJE BRZINE

        glUseProgram(progressBarShader);
        glBindVertexArray(VAO[4]);

        glm::mat4 transSpeedMax = glm::mat4(1.0f);
        transSpeedMax = glm::translate(transSpeedMax, glm::vec3(0.4f, 0.0f, 0));
        glUniformMatrix4fv(uniGasTrans, 1, GL_FALSE, glm::value_ptr(transSpeedMax));
        glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

        glm::mat4 transSpeed= glm::mat4(1.0f);
        transSpeed = glm::scale(transSpeed, glm::vec3(speedAmount, 1.0f, 1.0f));
        transSpeed = glm::translate(transSpeed, glm::vec3(0.3f, 0.0f, 0));
        glUniformMatrix4fv(uniGasTrans, 1, GL_FALSE, glm::value_ptr(transSpeed));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);


        // DISPLEJ SA KRUGOM

        glUseProgram(progressBarShader);
        glBindVertexArray(VAO[5]);

        glm::mat4 transDisplay = glm::mat4(1.0f);
        transDisplay = glm::translate(transDisplay, glm::vec3(0.4f, -0.4f, 0));
        glUniformMatrix4fv(uniDisplayTrans, 1, GL_FALSE, glm::value_ptr(transDisplay));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        // KRUG

        if (speed < 10)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glEnable(GL_PROGRAM_POINT_SIZE);
            glPointSize(4);
        }
        if (speed >= 10 && speed <= 30)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if ( speed > 30) {
            std::cout << "Yes\n";
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        
        glUseProgram(circleShader);
        glBindVertexArray(VAO[6]);

        glUniform1f(uCirclePosX, circlePositionX);

        glm::mat4 transCircle = glm::mat4(1.0f);
        transCircle = glm::translate(transCircle, glm::vec3(0.4f, -0.4f, 0));
        transCircle = glm::scale(transCircle, glm::vec3(1.0f, 1200.0f/600.0f, 1.0f));
        glUniformMatrix4fv(uniCircleTrans, 1, GL_FALSE, glm::value_ptr(transCircle));
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(circle) / (2 * sizeof(float)));

        // U svakoj iteraciji render petlje se krug pomera za step
        circlePositionX += circleStep;
        // ako krug dodje do leve ili desne granice obrce se znak step-a, znaci da se krece u suprotnom smeru od sledece iteracije
        if (circlePositionX >= circleBoundRight || circlePositionX <= circleBoundLeft)
            circleStep = -circleStep;


        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);


        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


        // POKAZIVACI PRAVCA

        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            if (!rightIndicatorOn)
                rightIndicatorOn = true;
            if (leftIndicatorOn)
                leftIndicatorOn = false;
            
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            if (!leftIndicatorOn)
                leftIndicatorOn = true;
            if (rightIndicatorOn) 
                rightIndicatorOn = false;
            
        }

        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            rightIndicatorOn = false;
            leftIndicatorOn = false;
        }

        if (car.getTurnRightSignal() || rightIndicatorOn) {
            glUseProgram(textureShader);
            glBindVertexArray(VAO[7]);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, rightIndicatorTexture);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

            glBindTexture(GL_TEXTURE_2D, 0);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            glUseProgram(0);
        }

        if (car.getTurnLeftSignal() || leftIndicatorOn) {
            glUseProgram(textureShader);
            glBindVertexArray(VAO[8]);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, leftIndicatorTexture);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

            glBindTexture(GL_TEXTURE_2D, 0);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            glUseProgram(0);
        }

        // IME

        glUseProgram(textureShader);
        glBindVertexArray(VAO[9]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, nameTexture);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

        glBindTexture(GL_TEXTURE_2D, 0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        glfwSwapBuffers(window);
    }

    endSimulation(&car);
    glDeleteBuffers(2, VBO);
    glDeleteVertexArrays(2, VAO);
    glDeleteProgram(textureShader);
    glfwTerminate();
    return 0;
}


// Autor funkcija: Nedeljko Tesanovic
unsigned int compileShader(GLenum type, const char* source)
{
    //Uzima kod u fajlu na putanji "source", kompajlira ga i vraca sejder tipa "type"
    //Citanje izvornog koda iz fajla
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
     std::string temp = ss.str();
     const char* sourceCode = temp.c_str(); //Izvorni kod sejdera koji citamo iz fajla na putanji "source"

    int shader = glCreateShader(type); //Napravimo prazan sejder odredjenog tipa (vertex ili fragment)
    
    int success; //Da li je kompajliranje bilo uspjesno (1 - da)
    char infoLog[512]; //Poruka o gresci (Objasnjava sta je puklo unutar sejdera)
    glShaderSource(shader, 1, &sourceCode, NULL); //Postavi izvorni kod sejdera
    glCompileShader(shader); //Kompajliraj sejder

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); //Provjeri da li je sejder uspjesno kompajliran
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog); //Pribavi poruku o gresci
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{
    //Pravi objedinjeni sejder program koji se sastoji od Vertex sejdera ciji je kod na putanji vsSource

    unsigned int program; //Objedinjeni sejder
    unsigned int vertexShader; //Verteks sejder (za prostorne podatke)
    unsigned int fragmentShader; //Fragment sejder (za boje, teksture itd)

    program = glCreateProgram(); //Napravi prazan objedinjeni sejder program

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource); //Napravi i kompajliraj vertex sejder
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource); //Napravi i kompajliraj fragment sejder

    //Zakaci verteks i fragment sejdere za objedinjeni program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program); //Povezi ih u jedan objedinjeni sejder program
    glValidateProgram(program); //Izvrsi provjeru novopecenog programa

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success); //Slicno kao za sejdere
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    //Posto su kodovi sejdera u objedinjenom sejderu, oni pojedinacni programi nam ne trebaju, pa ih brisemo zarad ustede na memoriji
    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}
static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Provjerava koji je format boja ucitane slike
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        // oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}