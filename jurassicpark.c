#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<X11/Xlib.h>
#include<X11/xpm.h>
#include<unistd.h>
#include<pthread.h>
#include"vroot.h"
#include"dennis.h"

#define N_IMAGE 45
#define N_COLORS 2

const char prompt[] = "> ";
const char permission_denied[] = "access: PERMISSION DENIED.";
const char permission_denied_and[] = "...and...";
const char nomagicword[] = "YOU DIDN'T SAY THE MAGIC WORD!";

pthread_mutex_t mutex;
Display *dpy;
Window root;
XWindowAttributes wa;
GC g;
XImage *img_array[N_IMAGE];
XImage *img;
char *colors[N_COLORS] = {"white", "blue"};
XColor xcolors[N_COLORS];
XColor xc, sc;              
int c;
XFontStruct *fs;
XGCValues v;
pthread_t img_thread;
pthread_cond_t animate_cond;
typedef enum { false, true } bool;
bool animate = false;

// dennis animation thread function
void *dennis_animation(void *arg){

    // refresh milliseconds rate
    int refresh_rate_millis = 70;    
    int img_index=0;

    // infinite loop
    while(1){
        pthread_mutex_lock(&mutex);
        while(!animate){
            img_index=-1;
            pthread_cond_wait(&animate_cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);
        
        // wait 0.75s before starting
        if(img_index==-1){
            img_index=0;
            usleep(750000);
        }

        // lock to prevent access to shared variables
        pthread_mutex_lock(&mutex);
        XPutImage(
            dpy,
            root,
            g,
            img_array[img_index],
            0, 0, 
            (wa.width-img_array[img_index]->width),
            0,
            img_array[img_index]->width,
            img_array[img_index]->height
        );
        XFlush(dpy);
        // unlock to allow access to shared variables
        pthread_mutex_unlock(&mutex);
        usleep(1000 * refresh_rate_millis);
        img_index = (img_index + 1) % N_IMAGE;
    }
}

// return a random number between 0 and 1
double rand_01(void){
    return rand() / ((double) RAND_MAX);
}

// simulate typing randomizing the time between character displaying
void simulate_typing(Display *dpy, Window root, GC g, int x, int y, char *command, int command_len, XFontStruct *fs){
    
    int orig_x = x;
    // print the prompt
    XDrawString(dpy, root, g, x, y, prompt, strlen(prompt));
    XFlush(dpy);
    // move x left by the prompt size
    x = x + XTextWidth(fs, prompt, strlen(prompt));
    // width of a string of one char
    int box_w = XTextWidth(fs, "?", strlen("?"));
    // draw cursor box
    XFillRectangle(dpy, root, g, x, y-fs->ascent, box_w, fs->descent+fs->ascent);
    XFlush(dpy);
    // wait before typing the command
    usleep(750000 + 1000000*rand_01());

    // used to store a string of one char at a time
    char temp[2];
    temp[1] = '\0';

    // print each command character
    for(int i=0; i<command_len; i++){

        // copy one char in a temp string
        temp[0] = command[i];
        // wait some randomized time at each character to simulate typing
        usleep(100000 + 10000 * rand_01());
        
        // set blue color
        XSetForeground(dpy, g, xcolors[1].pixel);
        // draw blue cursor box to cover the previous cursor
        XFillRectangle(dpy, root, g, x, y-fs->ascent, box_w, fs->descent+fs->ascent);
        XFlush(dpy);

        // set white color
        XSetForeground(dpy, g, xcolors[0].pixel);
        // draw the new char
        XDrawString(dpy, root, g, x, y, temp, strlen(temp));
        XFlush(dpy);
        
        // move the x to the left by the size of the string and print the new cursor
        x = x + XTextWidth(fs, temp, strlen(temp));
        XFillRectangle(dpy, root, g, x, y-fs->ascent, box_w, fs->descent+fs->ascent);
        XFlush(dpy);
    }
    
    /*-------- simulate ENTER hit --------*/
    // wait some time before hitting enter
    usleep(500000);
    
    // draw blue cursor box to cover the previous cursor
    XSetForeground(dpy, g, xcolors[1].pixel);
    XFillRectangle(dpy, root, g, x, y-fs->ascent, box_w, fs->descent+fs->ascent);
    XFlush(dpy);

    // move cursor to the row below
    // set white
    XSetForeground(dpy, g, xcolors[0].pixel);
    // move y to the row below
    y = y + fs->ascent + fs->descent;
    // draw new cursor
    XFillRectangle(dpy, root, g, orig_x, y-fs->ascent, box_w, fs->descent+fs->ascent);
    XFlush(dpy);
    // wait some time before proceeding to simulate computational time
    usleep(1000000 + 1000000 * rand_01());
    
    // remove cursor and proceed to the command output
    XSetForeground(dpy, g, xcolors[1].pixel);
    XFillRectangle(dpy, root, g, orig_x, y-fs->ascent, box_w, fs->descent+fs->ascent);
    XFlush(dpy);

    // restore white foreground
    XSetForeground(dpy, g, xcolors[0].pixel);
}

int main(){

    /************************* PARAMETER SETTING *************************/
    /* open the display (connect to the X server) */
    dpy = XOpenDisplay (getenv ("DISPLAY"));
    /* get the root window */
    root = DefaultRootWindow (dpy);
    /* get attributes of the root window */
    XGetWindowAttributes(dpy, root, &wa);
    /* create a GC for drawing in the window */
    g = XCreateGC (dpy, root, 0, NULL);

    /* allocate colors */
    for(int c=0; c<N_COLORS; c++) {
        XAllocNamedColor(
            dpy,
            DefaultColormapOfScreen(DefaultScreenOfDisplay(dpy)),
            colors[c],
            &sc,
            &xc);
        xcolors[c]=sc;
    }

    
    // char path[255];
    // char img_name[] = "frame_%d_delay-0.07s.XPM";
    // for(int i=0; i<N_IMAGE; i++){
        
    //     sprintf(path, "/home/marco/Desktop/jurassicpark-saver/imgs/frame_%d_delay-0.07s.XPM", i);
    //     if (XpmReadFileToImage (dpy, path, &img, NULL, NULL)) {
    //         printf("Error reading file %s\n", path);
    //         exit (1);
    //     }
    //     img_array[i] = malloc(sizeof(img));
    //     img_array[i] = img;
    // }



    // frame0
    if(XpmCreateImageFromData(dpy, frame0, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[0] = malloc(sizeof(img));
    img_array[0] = img;

    // frame1
    if(XpmCreateImageFromData(dpy, frame1, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[1] = malloc(sizeof(img));
    img_array[1] = img;

    // frame2
    if(XpmCreateImageFromData(dpy, frame2, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[2] = malloc(sizeof(img));
    img_array[2] = img;

    // frame3
    if(XpmCreateImageFromData(dpy, frame3, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[3] = malloc(sizeof(img));
    img_array[3] = img;

    // frame4
    if(XpmCreateImageFromData(dpy, frame4, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[4] = malloc(sizeof(img));
    img_array[4] = img;

    // frame5
    if(XpmCreateImageFromData(dpy, frame5, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[5] = malloc(sizeof(img));
    img_array[5] = img;

    // frame6
    if(XpmCreateImageFromData(dpy, frame6, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[6] = malloc(sizeof(img));
    img_array[6] = img;

    // frame7
    if(XpmCreateImageFromData(dpy, frame7, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[7] = malloc(sizeof(img));
    img_array[7] = img;

    // frame8
    if(XpmCreateImageFromData(dpy, frame8, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[8] = malloc(sizeof(img));
    img_array[8] = img;

    // frame9
    if(XpmCreateImageFromData(dpy, frame9, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[9] = malloc(sizeof(img));
    img_array[9] = img;

    // frame10
    if(XpmCreateImageFromData(dpy, frame10, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[10] = malloc(sizeof(img));
    img_array[10] = img;

    // frame11
    if(XpmCreateImageFromData(dpy, frame11, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[11] = malloc(sizeof(img));
    img_array[11] = img;

    // frame12
    if(XpmCreateImageFromData(dpy, frame12, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[12] = malloc(sizeof(img));
    img_array[12] = img;

    // frame13
    if(XpmCreateImageFromData(dpy, frame13, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[13] = malloc(sizeof(img));
    img_array[13] = img;

    // frame14
    if(XpmCreateImageFromData(dpy, frame14, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[14] = malloc(sizeof(img));
    img_array[14] = img;

    // frame15
    if(XpmCreateImageFromData(dpy, frame15, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[15] = malloc(sizeof(img));
    img_array[15] = img;

    // frame16
    if(XpmCreateImageFromData(dpy, frame16, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[16] = malloc(sizeof(img));
    img_array[16] = img;

    // frame17
    if(XpmCreateImageFromData(dpy, frame17, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[17] = malloc(sizeof(img));
    img_array[17] = img;

    // frame18
    if(XpmCreateImageFromData(dpy, frame18, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[18] = malloc(sizeof(img));
    img_array[18] = img;

    // frame19
    if(XpmCreateImageFromData(dpy, frame19, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[19] = malloc(sizeof(img));
    img_array[19] = img;

    // frame20
    if(XpmCreateImageFromData(dpy, frame20, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[20] = malloc(sizeof(img));
    img_array[20] = img;

    // frame21
    if(XpmCreateImageFromData(dpy, frame21, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[21] = malloc(sizeof(img));
    img_array[21] = img;

    // frame22
    if(XpmCreateImageFromData(dpy, frame22, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[22] = malloc(sizeof(img));
    img_array[22] = img;

    // frame23
    if(XpmCreateImageFromData(dpy, frame23, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[23] = malloc(sizeof(img));
    img_array[23] = img;

    // frame24
    if(XpmCreateImageFromData(dpy, frame24, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[24] = malloc(sizeof(img));
    img_array[24] = img;

    // frame25
    if(XpmCreateImageFromData(dpy, frame25, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[25] = malloc(sizeof(img));
    img_array[25] = img;

    // frame26
    if(XpmCreateImageFromData(dpy, frame26, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[26] = malloc(sizeof(img));
    img_array[26] = img;

    // frame27
    if(XpmCreateImageFromData(dpy, frame27, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[27] = malloc(sizeof(img));
    img_array[27] = img;

    // frame28
    if(XpmCreateImageFromData(dpy, frame28, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[28] = malloc(sizeof(img));
    img_array[28] = img;

    // frame29
    if(XpmCreateImageFromData(dpy, frame29, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[29] = malloc(sizeof(img));
    img_array[29] = img;

    // frame30
    if(XpmCreateImageFromData(dpy, frame30, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[30] = malloc(sizeof(img));
    img_array[30] = img;

    // frame31
    if(XpmCreateImageFromData(dpy, frame31, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[31] = malloc(sizeof(img));
    img_array[31] = img;

    // frame32
    if(XpmCreateImageFromData(dpy, frame32, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[32] = malloc(sizeof(img));
    img_array[32] = img;

    // frame33
    if(XpmCreateImageFromData(dpy, frame33, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[33] = malloc(sizeof(img));
    img_array[33] = img;

    // frame34
    if(XpmCreateImageFromData(dpy, frame34, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[34] = malloc(sizeof(img));
    img_array[34] = img;

    // frame35
    if(XpmCreateImageFromData(dpy, frame35, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[35] = malloc(sizeof(img));
    img_array[35] = img;

    // frame36
    if(XpmCreateImageFromData(dpy, frame36, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[36] = malloc(sizeof(img));
    img_array[36] = img;

    // frame37
    if(XpmCreateImageFromData(dpy, frame37, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[37] = malloc(sizeof(img));
    img_array[37] = img;

    // frame38
    if(XpmCreateImageFromData(dpy, frame38, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[38] = malloc(sizeof(img));
    img_array[38] = img;

    // frame39
    if(XpmCreateImageFromData(dpy, frame39, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[39] = malloc(sizeof(img));
    img_array[39] = img;

    // frame40
    if(XpmCreateImageFromData(dpy, frame40, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[40] = malloc(sizeof(img));
    img_array[40] = img;

    // frame41
    if(XpmCreateImageFromData(dpy, frame41, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[41] = malloc(sizeof(img));
    img_array[41] = img;

    // frame42
    if(XpmCreateImageFromData(dpy, frame42, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[42] = malloc(sizeof(img));
    img_array[42] = img;

    // frame43
    if(XpmCreateImageFromData(dpy, frame43, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[43] = malloc(sizeof(img));
    img_array[43] = img;

    // frame44
    if(XpmCreateImageFromData(dpy, frame44, &img, NULL, NULL)) {
        printf("Error reading image file\n");
        exit (1);
    }
    img_array[44] = malloc(sizeof(img));
    img_array[44] = img;









    /* load a font */
    Font f=XLoadFont(dpy, "-*-fixed-bold-*-*-*-15-*-*-*-*-*-*-*");
    XSetFont(dpy, g, f);

    /* get font metrics */
    XGetGCValues (dpy, g, GCFont, &v);
    fs = XQueryFont (dpy, v.font);

    pthread_cond_init (&animate_cond, NULL);
    pthread_mutex_init(&mutex, NULL);
    int rc = pthread_create(&img_thread, NULL, dennis_animation, "Starting animation...");
    if(rc){
        printf("ERROR: return code from pthread_create() was %d\n", rc);
        pthread_exit(NULL);
    }
    
    /************************* END PARAMETERS SETTING *************************/
    // infinite loop (xscreensaver breaks this)
    while(1){
        /* set blue background */
        XSetForeground(dpy, g, xcolors[1].pixel);
        XFillRectangle (dpy, root, g, 0, 0, wa.width, wa.height);
        XFlush(dpy);
        
        // set white color
        XSetForeground(dpy, g, xcolors[0].pixel);

        // wait a little before start showing
        usleep(300000);

        // x and y coords for text
        int x = 3;
        int y = 3 + fs->ascent;
        XDrawString(dpy, root, g, x, y, "Jurassic Park, System Security Interface", strlen("Jurassic Park, System Security Interface"));
        y = y + fs->ascent + fs->descent;
        XDrawString(dpy, root, g, x, y, "Version 4.0.5, Alpha E", strlen("Version 4.0.5, Alpha E"));
        XFlush(dpy);
        y = y + fs->ascent + fs->descent;
        usleep(1000000 + rand_01() * 500000);

        XDrawString(dpy, root, g, x, y, "Ready...", strlen("Ready..."));
        XFlush(dpy);
        y = y + fs->ascent + fs->descent;
        // End Headers

        // Access security attempts
        char command1[] = "access security";
        simulate_typing(dpy, root, g, x, y, command1, strlen(command1), fs);
        y = y + fs->ascent + fs->descent;
        //usleep(1000*500);
        
        // output access: PERMISSION DENIED.
        XDrawString(dpy, root, g, x, y, permission_denied, strlen(permission_denied));
        XFlush(dpy);
        y = y + fs->ascent + fs->descent;
        usleep(100000 + 100000 * rand_01());

        // command2
        char command2[] = "access security grid";
        // Here should go a routine that simulates typing
        simulate_typing(dpy, root, g, x, y, command2, strlen(command2), fs);
        y = y + fs->ascent + fs->descent;
        //usleep(1000*1000);

        // output command2
        XDrawString(dpy, root, g, x, y, permission_denied, strlen(permission_denied));
        XFlush(dpy);
        y = y + fs->ascent + fs->descent;
        usleep(100000 + 100000 * rand_01());
        
        // command3
        char command3[] = "access main security grid";
        simulate_typing(dpy, root, g, x, y, command3, strlen(command3), fs);
        y = y + fs->ascent + fs->descent;
        //usleep(1000*1500);
        // output command3
        XDrawString(dpy, root, g, x, y, permission_denied, strlen(permission_denied));
        XFlush(dpy);
        //y = y + fs->ascent + fs->descent;
        usleep(800000);

        int x1 = x + XTextWidth(fs, permission_denied, strlen(permission_denied));
        XDrawString(dpy, root, g, x1, y, permission_denied_and, strlen(permission_denied_and));
        XFlush(dpy);
        // wait before printing "you didn't say the right word!"
        usleep(1000000);

        // set flag that makes animation thread proceed and signal the animation thread
        pthread_mutex_lock(&mutex);
        animate = true;
        pthread_cond_signal(&animate_cond);
        pthread_mutex_unlock(&mutex);
        
        int end_of_screen_flag = 0;
        // print "you didn't say the right word!"
        y = y + fs->ascent + fs->descent;
        while(!end_of_screen_flag){

            pthread_mutex_lock(&mutex);
            XDrawString(dpy, root, g, x, y, nomagicword, strlen(nomagicword));
            pthread_mutex_unlock(&mutex);
            y = y + fs->ascent + fs->descent;
            XFlush(dpy);
            if(y > wa.height * 1.2){
                end_of_screen_flag = 1;
            }
            else{
                usleep(300000);
            }
        }
        
        // set flag to 
        pthread_mutex_lock(&mutex);
        animate = false;
        XClearWindow(dpy, root);
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}