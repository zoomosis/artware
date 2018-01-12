#if (!defined(DVAWARE_H))
#define DVAWARE_H

extern unsigned int inDV;               // flag indicating if running under
                                        // DESQview.  =0 not in DESQview,
                                        // =DESQview version number if in
                                        // DESQview.  hi = major version,
                                        // lo = minor version number.  Set
                                        // by dv_get_version


#if ((!defined(__cplusplus)) || (!defined(DVAWARE_INLINE)))

   #if (defined(__cplusplus))
      extern "C" {
   #endif
                                           // begin critical region,
                                           // no time slicing
   void dv_beginc(                            // procedure
      void                                    // no parameters
   );

                                           // sets up TC conio system to
                                           // be DESQview screen aware
                                           // calls dv_get_video_buffer
   void dv_conio(                             // procedure
      void                                    // no parameters
   );

                                           // end critical region,
                                           // allow time slicing
   void dv_endc(                              // procedure
      void                                    // no parameters
   );

                                           // Test to see if DV is running
   unsigned int dv_get_version(               // returns DV version or 0
      void                                    // no parameters
   );

                                           // get the video buffer segment
                                           // doesn't calls dv_get_version
   char _seg* dv_get_video_buffer(            // rets video buffer segment
      char _seg* assumed                      // assumed video buffer seg
   );

                                           // give up rest time slice
   void dv_pause(                             // procedure
      void                                    // no parameters
   );


   #if (defined(__cplusplus))
      };
   #endif

#else // defined(__cplusplus) && defined(DVAWARE_INLINE)

   #include <dos.h>
                                           // begin critical region,
                                           // no time slicing
   inline void dv_beginc(                     // procedure
      void                                    // no parameters
   ) {
      _AX = 0x101B;                        // BEGINC DV API call
      geninterrupt( 0x15 );                // DV API CALL
   }

                                           // end critical region,
                                           // allow time slicing
   inline void dv_endc(                       // procedure
      void                                    // no parameters
   ) {
      _AX = 0x101C;                        // ENDC DV API call
      geninterrupt( 0x15 );                // DV API CALL
   }

                                           // Test to see if DV is running
   inline unsigned int dv_get_version(        // returns DV version or 0
      void                                    // no parameters
   ) {
      _CX = 0x4445; // 'DE'              // set CX to 4445H; DX to 5351H
      _DX = 0x5351; // 'SQ'              // (an invalid date)
      _AX = 0x2B01;                        // DOS' set data function
      geninterrupt( 0x21 );                // call DOS
      if ( _AL != 0xFF )                   // if DOS didn't see as invalid
         _AX = _BX;                           // AH=major ver; AL=minor ver
      else                                 // DOS saw as invalid
         _AX = 0;                             // no desqview
      inDV = _AX;                          // Save version in inDV flag
      return _AX;                          // Return version number
   }

   //
   // Machine code to push and pop various registers.
   // implemented using #define instead of inline to avoid possible 
   // problems if Options/Compiler/C++ options.../Out-of-line inline 
   // functions turned on (or -vi isn't included in the command line 
   // compiler.
   //
   #define pushdi() __emit__( 0x57 )
   #define pushes() __emit__( 0x06 )
   #define popdi()  __emit__( 0x5F )
   #define popes()  __emit__( 0x07 )

                                           // get the video buffer segment
                                           // doesn't calls dv_get_version
   inline char _seg* dv_get_video_buffer(     // rets video buffer segment
      char _seg* assumed                      // assumed video buffer seg
   ) {
      pushdi();                            // save registers
      pushes();                            // save registers
      _ES = (unsigned int)assumed;         // get assumed segment into es
      _AH = 0xFE;                          // call get buffer
      geninterrupt( 0x10 );                // video BIOS call
      _AX = _ES;                           // put segment into accumulator
      popes();                             // restore registers
      popdi();                             // restore registers
      return (char _seg*)_AX;              // return segment
   }

   #undef pushdi() 
   #undef pushes() 
   #undef popdi()  
   #undef popes()  

                                           // give up rest time slice
   inline void dv_pause(                      // procedure
      void                                    // no parameters
   ) {
      _AX = 0x1000;                        // PAUSE DV API Call
      geninterrupt( 0x15 );                // DV API CALL
   }

                                           // sets up TC conio system to
                                           // be DESQview screen aware
                                           // calls dv_get_video_buffer
   inline void dv_conio(                      // procedure
      void                                    // no parameters
   ) {
      *(&directvideo-1) = (unsigned int)dv_get_video_buffer(
                                (char _seg*)(*(&directvideo-1)) );
   }

#endif // !defined(__cplusplus) || !defined(DVAWARE_INLINE)

#endif // DVAWARE_H

