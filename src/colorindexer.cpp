/*
 Component: Color Indexer

 Legend:
  [] ... array

 Input
  [Sensor R G B] ... message
  [White_Saturation_From 
   White_Saturation_To
   White_Value_From 
   White_Value_To 
   Gray_Saturation_From 
   Gray_Saturation_To 
   Gray_Value_From 
   Gray_Value_To 
   Black_Saturation_From 
   Black_Saturation_To
   Black_Value_From 
   Black_Value_To] ... message

 Output
  [Sensor H S V]
  [Sensor Color_ID]

 Predefined colors (Hue)
  #1      Red      0-21    
  #2      Orange   22-38
  ...

 Format
  Hue            0..359
  Saturation     0..100
  Value          0..100
  Red            0..255
  Green          0..255
  Value          0..255

*/

// external object's link to available Max functions
#include "ext.h"     

// this is the resource ID used in the resource file colorindexer.rsrc
#define RES_ID 10121

// white/black/gray options
#define WBG_LNG 12

// color id parameters (fromColor1, toColor1, fromColor2, ...)
static int colorIdHues={
    0,21,   // red
    22,28   // orange
    // ...
};

typedef struct colorindexer {	// defines our object's internal variables for each instance in a patch
    t_object c_ob;		// object header - ALL objects MUST begin with this...

    t_atom* c_rgbInput;		// received from the left inlet and stored internally for each object instance
    t_atom* c_optionsInput;	// received from the right inlet and stored internally for each object instance

    // outlet creation - inlets are automatic, but objects must "own" their own outlets

    // SensorHSV outlet
    void *c_hsv_outlet;			
    // SensorColorId outlet
    void *c_colorId_outlet;

    // color IDs hue data (white, black, gray)
    int wbg[12];

} t_colorindexer;

void *colorindexer_class;	// global pointer to the object class - so MAX can reference the object 

// these are prototypes for the methods that are defined below
void colorindexer_list(colorIndexer *x, t_symbol *msg, short argc, t_atom *argv);
// ...
// RGB X HSV transformations
void HSVToRGB( float H, float S, float V, float &R, float &G, float &B );
void RGBToHSV( float R, float G, float B, float &H, float &S, float &V );

// - methods --------------------------------------------------------------------------------------------------------------

void main(void) {
    setup((t_messlist **)&colorindexer_class, 
          (method)colorindexer_new, 
          0L, 
          (short)sizeof(t_colorindexer), 
          0L, 
          A_DEFLONG, 
          0); 

    /*
     In most cases, you'll want the leftmost inlet to cause the object to output value or
     perform some kind of action, while the other inlets are used to store additional
     information needed when the action is taken.
    */

    // inlet handler(s)
    addmess((method)colorindexer_list, "list", A_GIMME, 0);   // handler for list messages about to be get from inlets

    post("Colorindexer object loaded...");	              // debug
}

/**
 * Create new instance of the object.
 */
void *colorindexer_new() { // n = int argument typed into object box (A_DEFLONG) -- defaults to 0 if no args are typed
    t_colorindexer *x;

    x=(t_colorindexer *)newobject(colorindexer_class);

    // create inlets - there will be two for list messages
    inlet_new(x,"list"); // inlet #1; free is made by MAX
    inlet_new(x,"list"); // inlet #2; free is made by MAX

    // create outlets (freed by MAX) - two outlets for list messages
    x->c_hsv_outlet=listout(x);
    x->c_colorId_outlet=listout(x);

    post("New instance of colorindexer %p...",x);

    return(x);
}

/**
 * Destroy the object.
 */
void colorindexer_free(t_colorindexer *x) {
    post("Free instance of colorindexer %p...",x);
}

/** 
 * Method handling the list message (handler is registered by addmess method within the main).
 *
 * t_symbol argument is unimportant and should be ignored.
 *
 * Params:
 *  msg  Name of the message received.
 *  argc Number of values (t_atoms) in the argv array.
 *  argv Array of t_atoms containing the list.
 */
void colorindexer_list(colorindexer *x, t_symbol *msg, short argc, t_atom *argv) {
    post("Handling list message: %d numbers",argc);
    short i;

    if(argc==4) {
        post(" Detected message Sensor/R/G/B");

        // process input
        long sensor=argv[0].a_w.w_long;
        float r,g,b,h,s,v;
        r=(float)argv[1].a_w.w_long/100.0;
        g=(float)argv[2].a_w.w_long/100.0;
        b=(float)argv[3].a_w.w_long/100.0;
        RGBToHSV(r,g,b,h,s,v);

        // HSV outlet
        #define HSV_LNG 4
        t_atom hsvList[HSV_LNG];
        long hsvNumbers[HSV_LNG];

        s*=100;
        v*=100;

        hsvNumbers[0] = sensor; // sensor
        hsvNumbers[1] = (long)h; // H
        hsvNumbers[2] = (long)s; // S
        hsvNumbers[3] = (long)v; // V

        for (i=0; i < HSV_LNG; i++) {
            SETLONG(hsvList+i,hsvNumbers[i]); // macro for setting a t_atom
        }
        outlet_list(x->c_hsv_outlet,0L,HSV_LNG,&hsvList);

        // Color ID outlet
        #define COLOR_ID_LNG 2
        t_atom colorIdList[COLOR_ID_LNG];
        long colorIdNumbers[COLOR_ID_LNG];

        // cycle to determine color ID - detect overflow

        colorIdNumbers[0] = sensor; // sensor
        colorIdNumbers[1] = 1; // color ID TODO fill in

        for (i=0; i < COLOR_ID_LNG; i++) {
            SETLONG(colorIdList+i,colorIdNumbers[i]); // macro for setting a t_atom
        }
        outlet_list(x->c_colorId_outlet,0L,COLOR_ID_LNG,&colorIdList);
    }
    else {
        if(argc==10) {
            post(" Detected options message Sensor/...");

            // todo: transfer options from the message to the object instance above

            post(" NO HANDLING AT THIS TIME");
        }
        else {
            post(" Unhandled message - wrong length - %d numbers...",argc);
        }
    }

    // TODO: decide according to the length of the message which one it was

    /*
    for (i=0; i < argc; i++) {
        switch (argv[i].a_type) {
            case A_LONG:
                post("argument %ld is a long: %ld", (long)i,
                argv[i].a_w.w_long);
            break;
            case A_SYM:
                post("argument %ld is a symbol: name %s", (long)i,
                argv[i].a_w.w_sym->s_name);
            break;
            case A_FLOAT:
                post("argument %ld is a float: %lf", (long)i,
                argv[i].a_w.w_float);
            break;
        }
    }
    */
}

// - rgb2hsv --------------------------------------------------------------------------------------------------------------

/*
 R ... <0,1>
 G ... <0,1>
 B ... <0,1>
 H ... <0,360>       zakladni spektralni barva
 S ... <0,1>         bila - spektralni
 V ... <0,1>         cerna - nejjasnejsi

 hue ... barevny odstin
	 H - zakladni spektralni barva - rozsah 0 -360 stupnu
 saturation ... sytost
         S - sytost, cistota - pomer ciste barvy a bile
         rozsah 0 ( bila ) az 1 ( spektralni )
 value ... jas
         V - jas, intenzita - rozsah 0 ( cerna ) az 1
*/

float minimum( float R, float G, float B )
{
 float min = R;
 if( G<min ) min=G;
 if( B<min ) return B; else return min;
}

float maximum( float R, float G, float B )
{
 float max = R;
 if( G>max ) max=G;
 if( B>max ) return B; else return max;
}

void RGBToHSV( float R, float G, float B, float &H, float &S, float &V ) {
 float max, min, delta;

 min = minimum( R, G, B );
 max = maximum( R, G, B );
 V = max;
 delta = max-min;
 if( max != 0.0 )
  S=delta/max;
 else
  S=0.0;

 if( delta != 0.0 )
 {
  if( R == max )
   H = ( G-B )/delta;
  else
   {
    if( G == max )
     H=2.0+(B-R)/delta;
    else
     H=4.0+(R-G)/delta;

    H*=60.0; // conversion to degrees
   }

  if( H<0.0 )
   H+=360.0;
 }
}

void HSVToRGB( float H, float S, float V, float &R, float &G, float &B ) {
 float i, f, p, q, t;

 if( S==0.0 )		// achromaticky pripad
 {
  R=V; G=V; B=V;
 }
 else			// chromaticky pripad
 {
  if( H==360.0 ) H=0.0;
  H/=60.0; 		// 0 <= H < 6
  i=(int)H;		// cislo vysece 0 <= i <= 5
  f=H-i;		// 0 <= f < 1

  p=V*(1.0-S);
  q=V*(1.0-S*f);
  t=V*(1.0-S*(1.0-f));

  switch( (int)i )  // sest vyseci
  {
   case 0:
	  R=V;
	  G=t;
	  B=p;
	  break;
   case 1:
	  R=q;
	  G=V;
	  B=p;
	  break;
   case 2:
	  R=p;
	  G=V;
	  B=t;
	  break;
   case 3:
	  R=p;
	  G=q;
	  B=V;
	  break;
   case 4:
	  R=t;
	  G=p;
	  B=V;
	  break;
   case 5:
	  R=V;
	  G=p;
	  B=q;
	  break;
  }
 }
}

// - eof -
