#include <fx2regs.h>
#include <fx2macros.h>
#include <delay.h>
#include <usbjt.h>
#include <i2c.h>
#include <setupdat.h>

#ifdef DEBUG_FIRMWARE
 #include <stdio.h>
#else
#define printf(...)
#endif

#define SYNCDELAY SYNCDELAY4

#define LED_ADDR		0x21

enum {
    Alt0_BulkIN = 0,
    Alt1_BulkOUT,
    Alt2_BulkINOUT,
    Alt3_IsocIN,
    Alt4_IsocOUT,
    Alt5_IsocIN,
    Alt6_IsocINOUT
};

enum {
    Full_Alt0_BulkIN = 0,
    Full_Alt1_BulkOUT,
    Full_Alt2_IsocIN,
    Full_Alt3_IsocOUT
};

BYTE xdata Digit[] = { 0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x98, 0x88, 0x83, 0xc6, 0xa1, 0x86, 0x8e };

BYTE    Configuration;      // Current configuration
BYTE    AlternateSetting = Alt0_BulkIN;   // Alternate settings


//-----------------------------------------------------------------------------
// Task Dispatcher hooks
//   The following hooks are called by the task dispatcher.
//-----------------------------------------------------------------------------

WORD mycount;

void main_init(void)             // Called once at startup
{
   int i,j;

   // set the CPU clock to 48MHz
//   CPUCS = ((CPUCS & ~bmCLKSPD) | bmCLKSPD1) ;
//   SYNCDELAY;

   // set the slave FIFO interface to 48MHz
   IFCONFIG |= 0x40;
   SYNCDELAY;

    // Default interface uses endpoint 2, zero the valid bit on all others
    // Just using endpoint 2, zero the valid bit on all others
    EP1OUTCFG = (EP1OUTCFG & 0x7F);
	SYNCDELAY;
	EP1INCFG = (EP1INCFG & 0x7F);
	SYNCDELAY;
	EP4CFG = (EP4CFG & 0x7F);
	SYNCDELAY;
	EP6CFG = (EP6CFG & 0x7F);
	SYNCDELAY;
	EP8CFG = (EP8CFG & 0x7F);
	SYNCDELAY;
    EP2CFG = 0xE0;  // EP2 is DIR=IN, TYPE=BULK, SIZE=512, BUF=4x

   // We want to get SOF interrupts
   ENABLE_SOF(); 

   mycount = 0;

   // Prepare data
   for (i=1;i<5;i++)
   {
      EP2FIFOBUF[0] = LSB(mycount);
      EP2FIFOBUF[1] = MSB(mycount);
      EP2FIFOBUF[2] = USBFRAMEL;
      EP2FIFOBUF[3] = USBFRAMEH;
      EP2FIFOBUF[4] = MICROFRAME;
      for (j=5;j<1024;j++)
      {
         EP2FIFOBUF[j] = i;
      }
      EP2BCH = 0x02;
      EP2BCL = 0x00;
   }


    remote_wakeup_allowed = TRUE;                 // Enable remote-wakeup
}

void main_loop(void)             // Called repeatedly while the device is idle
{  
	// ...FX2 in high speed mode
	if( HISPEED )
	{ 
	    // Perform USB activity based upon the Alt. Interface selected 
	     switch (AlternateSetting)
	    {
	        case Alt0_BulkIN:
	            // Send data on EP2
	            if(!(EP2468STAT & bmEP2FULL))
	            {
	                EP2FIFOBUF[0] = LSB(mycount);
	                EP2FIFOBUF[1] = MSB(mycount);
	                EP2FIFOBUF[2] = USBFRAMEL;
	                EP2FIFOBUF[3] = USBFRAMEH;
	                EP2FIFOBUF[4] = MICROFRAME;
	
	                EP2BCH = 0x02;
	                EP2BCL = 0x00;
	
	                mycount++;
	            }
	        break;
	
	        case Alt2_BulkINOUT:
	            // Send data on EP2
	            if(!(EP2468STAT & bmEP2FULL))
	            {
	                EP2FIFOBUF[0] = LSB(mycount);
	                EP2FIFOBUF[1] = MSB(mycount);
	                EP2FIFOBUF[2] = USBFRAMEL;
	                EP2FIFOBUF[3] = USBFRAMEH;
	                EP2FIFOBUF[4] = MICROFRAME;
	
	                EP2BCH = 0x02;
	                EP2BCL = 0x00;
	
	                mycount++;
	            }
	
	            // check EP6 EMPTY(busy) bit in EP2468STAT (SFR), core set's this bit when FIFO is empty
	            if(!(EP2468STAT & bmEP6EMPTY))
	            { 
	                EP6BCL = 0x80;          // re(arm) EP6OUT
	            }
	        break;
	
	        case Alt3_IsocIN:
	        case Alt5_IsocIN:
	            // Send data on EP2
	            if(!(EP2468STAT & bmEP2FULL))
	            {
	                EP2FIFOBUF[0] = LSB(mycount);
	                EP2FIFOBUF[1] = MSB(mycount);
	                EP2FIFOBUF[2] = USBFRAMEL;
	                EP2FIFOBUF[3] = USBFRAMEH;
	                EP2FIFOBUF[4] = MICROFRAME;
	
	                EP2BCH = 0x04;
	                EP2BCL = 0x00;
	
	                mycount++;
	            }
	        break;
	
	
	        case Alt1_BulkOUT:
	        case Alt4_IsocOUT:
	            // check EP2 EMPTY(busy) bit in EP2468STAT (SFR), core set's this bit when FIFO is empty
	            if(!(EP2468STAT & bmEP2EMPTY))
	            { 
	                EP2BCL = 0x80;          // re(arm) EP2OUT
	            }
	        break;
	
	        case Alt6_IsocINOUT:
	        {
	            // Send data on EP2
	            if(!(EP2468STAT & bmEP2FULL))
	            {
	                EP2FIFOBUF[0] = LSB(mycount);
	                EP2FIFOBUF[1] = MSB(mycount);
	                EP2FIFOBUF[2] = USBFRAMEL;
	                EP2FIFOBUF[3] = USBFRAMEH;
	                EP2FIFOBUF[4] = MICROFRAME;
	
	                EP2BCH = 0x04;
	                EP2BCL = 0x00;
	
	                mycount++;
	            }
	
	            // check EP6 EMPTY(busy) bit in EP2468STAT (SFR), core set's this bit when FIFO is empty
	            if(!(EP2468STAT & bmEP6EMPTY))
	            { 
	                EP6BCL = 0x80;          // re(arm) EP6OUT
	            }
	        }
	        break;
	   }
	   
	}
	else	// Full Speed
	{
	    // Perform USB activity based upon the Alt. Interface selected 
	     switch (AlternateSetting)
	    {
	        case Full_Alt0_BulkIN:
	            // Send data on EP2
	            if(!(EP2468STAT & bmEP2FULL))
	            {
	                EP2FIFOBUF[0] = LSB(mycount);
	                EP2FIFOBUF[1] = MSB(mycount);
	                EP2FIFOBUF[2] = USBFRAMEL;
	                EP2FIFOBUF[3] = USBFRAMEH;
	                EP2FIFOBUF[4] = MICROFRAME;
	
	                EP2BCH = 0x00;
	                EP2BCL = 0x40;
	
	                mycount++;
	            }
	        break;
	
	        case Full_Alt1_BulkOUT:	
	            // check EP2 EMPTY(busy) bit in EP2468STAT (SFR), core set's this bit when FIFO is empty
	            if(!(EP2468STAT & bmEP2EMPTY))
	            { 
	                EP2BCL = 0x80;          // re(arm) EP2OUT
	            }
	        break;
	
	        case Full_Alt2_IsocIN:
	            // Send data on EP2
	            if(!(EP2468STAT & bmEP2FULL))
	            {
	                EP2FIFOBUF[0] = LSB(mycount);
	                EP2FIFOBUF[1] = MSB(mycount);
	                EP2FIFOBUF[2] = USBFRAMEL;
	                EP2FIFOBUF[3] = USBFRAMEH;
	                EP2FIFOBUF[4] = MICROFRAME;
	
	                EP2BCH = 0x03;	// 1023
	                EP2BCL = 0xFF;
	
	                mycount++;
	            }
	        break;
	
	        case Full_Alt3_IsocOUT:
	            // check EP2 EMPTY(busy) bit in EP2468STAT (SFR), core set's this bit when FIFO is empty
	            if(!(EP2468STAT & bmEP2EMPTY))
	            { 
	                EP2BCL = 0x80;          // re(arm) EP2OUT
	            }
	        break;
	   }	
	}
}


//-----------------------------------------------------------------------------
// Device Request hooks
//   The following hooks are called by the end point 0 device request parser.
//-----------------------------------------------------------------------------

BOOL handle_set_configuration(BYTE cfg)   // Called when a Set Configuration command is received
{
   Configuration = cfg;
   return(TRUE);            // Handled by user code
}

BYTE handle_get_configuration ()   // Called when a Get Configuration command is received
{
    return Configuration;
}


BOOL handle_vendorcommand(BYTE cmd) {
 printf ( "Vendor Command %u\n", cmd );
 return FALSE;
}

BOOL handle_get_interface(BYTE ifc, BYTE* alt) {
 *alt=AlternateSetting;
 return TRUE;
}

BOOL handle_set_interface(BYTE ifc, BYTE alt)       // Called when a Set Interface command is received
{
    BYTE	updateDisplay = TRUE;
    AlternateSetting = alt;

	// ...FX2 in high speed mode
	if( HISPEED )
	{ 
	    // Change configuration based upon the Alt. Interface selected 
	    switch (AlternateSetting)
	    {
	        case Alt0_BulkIN:
	            // Only using endpoint 2, zero the valid bit on all others
	            // Just using endpoint 2, zero the valid bit on all others
	            EP2CFG = 0xE0;  // EP2 is DIR=IN, TYPE=BULK, SIZE=512, BUF=4x
	            SYNCDELAY;
	
	            EP1OUTCFG = (EP1OUTCFG & 0x7F);
	        	SYNCDELAY;
	        	EP1INCFG = (EP1INCFG & 0x7F);
	        	SYNCDELAY;
	        	EP4CFG = (EP4CFG & 0x7F);
	        	SYNCDELAY;
	        	EP6CFG = (EP6CFG & 0x7F);
	        	SYNCDELAY;
	        	EP8CFG = (EP8CFG & 0x7F);
	        	SYNCDELAY;
	
	            // Clear out any committed packets
	            FIFORESET = 0x80;
	            SYNCDELAY;
	            FIFORESET = 0x02;
	            SYNCDELAY;
	            FIFORESET = 0x00;
	            SYNCDELAY;
	
	            // Reset data toggle to 0
	            TOGCTL = 0x12;  // EP2 IN
	            TOGCTL = 0x32;  // EP2 IN Reset
	
	        break;
	
	        case Alt1_BulkOUT:
	            // Only using endpoint 2, zero the valid bit on all others
	            EP2CFG = 0xA0;  // EP2 is DIR=OUT, TYPE=BULK, SIZE=512, BUF=4x
	            SYNCDELAY;
	
	            EP1OUTCFG = (EP1OUTCFG & 0x7F);
	        	SYNCDELAY;
	        	EP1INCFG = (EP1INCFG & 0x7F);
	        	SYNCDELAY;
	        	EP4CFG = (EP4CFG & 0x7F);
	        	SYNCDELAY;
	        	EP6CFG = (EP6CFG & 0x7F);
	        	SYNCDELAY;
	        	EP8CFG = (EP8CFG & 0x7F);
	        	SYNCDELAY;
	           
	            // OUT endpoints do NOT come up armed
	            EP2BCL = 0x80; // arm first buffer by writing BC w/skip=1
	            SYNCDELAY;
	            EP2BCL = 0x80; // arm second buffer by writing BC w/skip=1
	            SYNCDELAY;
	            EP2BCL = 0x80; // arm third buffer by writing BC w/skip=1
	            SYNCDELAY;
	            EP2BCL = 0x80; // arm fourth buffer by writing BC w/skip=1
	            
	        break;
	
	        case Alt2_BulkINOUT:
	            // Using endpoints 2 & 6, zero the valid bit on all others
	            EP2CFG = 0xE2; // EP2 is DIR=IN, TYPE=BULK, SIZE=512, BUF=2x
	        	SYNCDELAY;
	            EP6CFG = 0xA2; // EP6 is DIR=OUT, TYPE=BULK, SIZE=512, BUF=2x   
	        	SYNCDELAY;
	            
	            EP1OUTCFG = (EP1OUTCFG & 0x7F);
	        	SYNCDELAY;
	        	EP1INCFG = (EP1INCFG & 0x7F);
	        	SYNCDELAY;
	        	EP4CFG = (EP4CFG & 0x7F);
	        	SYNCDELAY;
	        	EP8CFG = (EP8CFG & 0x7F);
	        	SYNCDELAY;
	
	            // Clear out any committed packets
	            FIFORESET = 0x80;
	            SYNCDELAY;
	            FIFORESET = 0x02;
	            SYNCDELAY;
	            FIFORESET = 0x00;
	            SYNCDELAY;
	
	            // Reset data toggle to 0
	            TOGCTL = 0x12;  // EP2 IN
	            TOGCTL = 0x32;  // EP2 IN Reset
	
	            // OUT endpoints do NOT come up armed
	            EP6BCL = 0x80; // arm first buffer by writing BC w/skip=1
	            SYNCDELAY;
	            EP6BCL = 0x80; // arm second buffer by writing BC w/skip=1
	        	SYNCDELAY;
	        break;
	
	        case Alt3_IsocIN:
	            // Only using endpoint 2, zero the valid bit on all others
	            EP2CFG = 0xD8;  // EP2 is DIR=IN, TYPE=ISOC, SIZE=1024, BUF=4x
	            SYNCDELAY;
	            
	            EP1OUTCFG = EP1INCFG = EP4CFG = EP6CFG = EP8CFG = 0x00; 
	            SYNCDELAY;
	
	            // Clear out any committed packets
	            FIFORESET = 0x80;
	            SYNCDELAY;
	            FIFORESET = 0x02;
	            SYNCDELAY;
	            FIFORESET = 0x00;
	            SYNCDELAY;
	
	            // This register sets the number of Isoc packets to send per
	            // uFrame.  This register is only valid in high speed.
	            EP2ISOINPKTS = 0x03;
	
	        break;
	
	        case Alt4_IsocOUT:
	        {
	            // Only using endpoint 2, zero the valid bit on all others
	            EP1OUTCFG = EP1INCFG = EP4CFG = EP6CFG = EP8CFG = 0x00; 
	            SYNCDELAY;
	            EP2CFG = 0x98;  // EP2 is DIR=OUT, TYPE=ISOC, SIZE=1024, BUF=4x
	            SYNCDELAY;
	
	            // OUT endpoints do NOT come up armed
	            EP2BCL = 0x80; // arm first buffer by writing BC w/skip=1
	            SYNCDELAY;
	            EP2BCL = 0x80; // arm second buffer by writing BC w/skip=1        break;
	
	        }
	        break;
	
	        case Alt5_IsocIN:
	        {
	            // Only using endpoint 2, zero the valid bit on all others
	            EP2CFG = 0xD8;  // EP2 is DIR=IN, TYPE=ISOC, SIZE=1024, BUF=4x
	            SYNCDELAY;
	
	            EP1OUTCFG = EP1INCFG = EP4CFG = EP6CFG = EP8CFG = 0x00; 
	            SYNCDELAY;
	
	            // Clear out any committed packets
	            FIFORESET = 0x80;
	            SYNCDELAY;
	            FIFORESET = 0x02;
	            SYNCDELAY;
	            FIFORESET = 0x00;
	            SYNCDELAY;
	
	            // This register sets the number of Isoc packets to send per
	            // uFrame.  This register is only valid in high speed.
	            EP2ISOINPKTS = 0x01;
	        }
	        break;
	
	        case Alt6_IsocINOUT:
	        {
	            // Using endpoints 2 & 6, zero the valid bit on all others
	            EP2CFG = 0xDA; // EP2 is DIR=IN, TYPE=ISOC, SIZE=1024, BUF=2x
	            SYNCDELAY;
	            EP6CFG = 0x9A; // EP6 is DIR=OUT, TYPE=ISOC, SIZE=1024, BUF=2x   
	            SYNCDELAY;
	
	            EP1OUTCFG = EP1INCFG = EP4CFG = EP8CFG = 0x00; 
	            SYNCDELAY;
	
	            // Clear out any committed packets
	            FIFORESET = 0x80;
	            SYNCDELAY;
	            FIFORESET = 0x02;
	            SYNCDELAY;
	            FIFORESET = 0x00;
	            SYNCDELAY;
	
	            // This register sets the number of Isoc packets to send per
	            // uFrame.  This register is only valid in high speed.
	            EP2ISOINPKTS = 0x01;
	
	            // OUT endpoints do NOT come up armed
	            EP6BCL = 0x80; // arm first buffer by writing BC w/skip=1
	            SYNCDELAY;
	            EP6BCL = 0x80; // arm second buffer by writing BC w/skip=1
	 
	        }
	        break;
	    }
	}
    else
    {
	    // Change configuration based upon the Alt. Interface selected 
	    switch (AlternateSetting)
	    {
	        case Full_Alt0_BulkIN:
	            // Only using endpoint 2, zero the valid bit on all others
	            // Just using endpoint 2, zero the valid bit on all others
	            EP2CFG = 0xE0;  // EP2 is DIR=IN, TYPE=BULK, SIZE=512, BUF=4x
	            SYNCDELAY;
	
	            EP1OUTCFG = (EP1OUTCFG & 0x7F);
	        	SYNCDELAY;
	        	EP1INCFG = (EP1INCFG & 0x7F);
	        	SYNCDELAY;
	        	EP4CFG = (EP4CFG & 0x7F);
	        	SYNCDELAY;
	        	EP6CFG = (EP6CFG & 0x7F);
	        	SYNCDELAY;
	        	EP8CFG = (EP8CFG & 0x7F);
	        	SYNCDELAY;
	
	            // Clear out any committed packets
	            FIFORESET = 0x80;
	            SYNCDELAY;
	            FIFORESET = 0x02;
	            SYNCDELAY;
	            FIFORESET = 0x00;
	            SYNCDELAY;
	
	            // Reset data toggle to 0
	            TOGCTL = 0x12;  // EP2 IN
	            TOGCTL = 0x32;  // EP2 IN Reset
	
	        break;
	
	        case Full_Alt1_BulkOUT:
	            // Only using endpoint 2, zero the valid bit on all others
	            EP2CFG = 0xA0;  // EP2 is DIR=OUT, TYPE=BULK, SIZE=512, BUF=4x
	            SYNCDELAY;
	
	            EP1OUTCFG = (EP1OUTCFG & 0x7F);
	        	SYNCDELAY;
	        	EP1INCFG = (EP1INCFG & 0x7F);
	        	SYNCDELAY;
	        	EP4CFG = (EP4CFG & 0x7F);
	        	SYNCDELAY;
	        	EP6CFG = (EP6CFG & 0x7F);
	        	SYNCDELAY;
	        	EP8CFG = (EP8CFG & 0x7F);
	        	SYNCDELAY;
	           
	            // OUT endpoints do NOT come up armed
	            EP2BCL = 0x80; // arm first buffer by writing BC w/skip=1
	            SYNCDELAY;
	            EP2BCL = 0x80; // arm second buffer by writing BC w/skip=1
	            SYNCDELAY;
	            EP2BCL = 0x80; // arm third buffer by writing BC w/skip=1
	            SYNCDELAY;
	            EP2BCL = 0x80; // arm fourth buffer by writing BC w/skip=1
	            
	        break;
	
	
	        case Full_Alt2_IsocIN:
	            // Only using endpoint 2, zero the valid bit on all others
	            EP2CFG = 0xD8;  // EP2 is DIR=IN, TYPE=ISOC, SIZE=1024, BUF=4x
	            SYNCDELAY;
	            
	            EP1OUTCFG = EP1INCFG = EP4CFG = EP6CFG = EP8CFG = 0x00; 
	            SYNCDELAY;
	
	            // Clear out any committed packets
	            FIFORESET = 0x80;
	            SYNCDELAY;
	            FIFORESET = 0x02;
	            SYNCDELAY;
	            FIFORESET = 0x00;
	            SYNCDELAY;
		
	        break;
	
	        case Full_Alt3_IsocOUT:
	        {
	            // Only using endpoint 2, zero the valid bit on all others
	            EP1OUTCFG = EP1INCFG = EP4CFG = EP6CFG = EP8CFG = 0x00; 
	            SYNCDELAY;
	            EP2CFG = 0x98;  // EP2 is DIR=OUT, TYPE=ISOC, SIZE=1024, BUF=4x
	            SYNCDELAY;
	
	            // OUT endpoints do NOT come up armed
	            EP2BCL = 0x80; // arm first buffer by writing BC w/skip=1
	            SYNCDELAY;
	            EP2BCL = 0x80; // arm second buffer by writing BC w/skip=1        break;
	
	        }
	        break;	
	
	    }
	}

   // Update the display to indicate the currently selected alt. Interface
	if(updateDisplay)
	{
	   eeprom_write(LED_ADDR, 0x01, 1, &(Digit[AlternateSetting]));
	   updateDisplay = FALSE;
	}

   return(TRUE);            // Handled by user code
}

//-----------------------------------------------------------------------------
// USB Interrupt Handlers
//   The following functions are called by the USB interrupt jump table.
//-----------------------------------------------------------------------------
void sof_isr() interrupt SOF_ISR {
 CLEAR_SOF();
}
void sutok_isr() interrupt SUTOK_ISR {}
void ep0ack_isr() interrupt EP0ACK_ISR {}
void ep0in_isr() interrupt EP0IN_ISR {}
void ep0out_isr() interrupt EP0OUT_ISR {}
void ep1in_isr() interrupt EP1IN_ISR {}
void ep1out_isr() interrupt EP1OUT_ISR {}
void ep2_isr() interrupt EP2_ISR {
    // Perform USB activity based upon the Alt. Interface selected 
     switch (AlternateSetting)
    {
        case Alt1_BulkOUT:
        case Alt4_IsocOUT:
           // check EP2 EMPTY(busy) bit in EP2468STAT (SFR), core set's this bit when FIFO is empty
            if(!(EP2468STAT & bmEP2EMPTY))
            { 
                EP2BCL = 0x80;          // re(arm) EP2OUT
            }
        break;

        case Alt2_BulkINOUT:
        case Alt6_IsocINOUT:
            // check EP6 EMPTY(busy) bit in EP2468STAT (SFR), core set's this bit when FIFO is empty
            if(!(EP2468STAT & bmEP6EMPTY))
            { 
                EP6BCL = 0x80;          // re(arm) EP6OUT
            }
        break;
   }

}
void ep4_isr() interrupt EP4_ISR {}
void ep6_isr() interrupt EP6_ISR {}
void ep8_isr() interrupt EP8_ISR {}
void ibn_isr() interrupt IBN_ISR {}
void ep0ping_isr() interrupt EP0PING_ISR {}
void ep1ping_isr() interrupt EP1PING_ISR {}
void ep2ping_isr() interrupt EP2PING_ISR {}
void ep4ping_isr() interrupt EP4PING_ISR {}
void ep6ping_isr() interrupt EP6PING_ISR {}
void ep8ping_isr() interrupt EP8PING_ISR {}
void errlimit_isr() interrupt ERRLIMIT_ISR {}
void ep2isoerr_isr() interrupt EP2ISOERR_ISR {}
void ep4isoerr_isr() interrupt EP4ISOERR_ISR {}
void ep6isoerr_isr() interrupt EP6ISOERR_ISR {}
void ep8isoerr_isr() interrupt EP8ISOERR_ISR {}
void spare_isr() interrupt RESERVED_ISR {}
void ep2pf_isr() interrupt EP2PF_ISR{}
void ep4pf_isr() interrupt EP4PF_ISR{}
void ep6pf_isr() interrupt EP6PF_ISR{}
void ep8pf_isr() interrupt EP8PF_ISR{}
void ep2ef_isr() interrupt EP2EF_ISR{}
void ep4ef_isr() interrupt EP4EF_ISR{}
void ep6ef_isr() interrupt EP6EF_ISR{}
void ep8ef_isr() interrupt EP8EF_ISR{}
void ep2ff_isr() interrupt EP2FF_ISR{}
void ep4ff_isr() interrupt EP4FF_ISR{}
void ep6ff_isr() interrupt EP6FF_ISR{}
void ep8ff_isr() interrupt EP8FF_ISR{}
void gpifdone_isr() interrupt GPIFDONE_ISR{}
void gpifwf_isr() interrupt GPIFWF_ISR{}


