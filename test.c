This is test file for merge
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "main2.h"
#include <stdio.h>
#include <string.h>
#include "system.h"
#include "flash.h"
#include "my_can.h"
#include "dac.h"

unsigned long ch_time123=0;
unsigned long all_ch_time=0;
unsigned long all_ch_I=0;
unsigned long all_ch_mode=0;
unsigned char led_block=0;

// this is part master
/// Определение значение макс. напряжений и температур
void CheckMaxBank(void)
{
		unsigned long uv=0;
		long tsr=0;
		char ctsr=0;
		BMS_Param.Umax=0;
		if((BMS_Param.ch_status.Charge_stage!=0x03)||((BMS_Param.ch_status.lastPreBalancing+15L*1000L)>HAL_GetTick()))BMS_Param.Umin=5000;
		BMS_Param.Tmax=0;
		BMS_Param.Tmin=500;
		for(int j=0;j<BMS_Param.BP.CellCount;j++){
					uv=uv+cellVoltage[j];
          if(cellVoltage[j]>BMS_Param.Umax){
						BMS_Param.Umax=cellVoltage[j];
            BMS_Param.CUmax=j+1;
					}
					if((BMS_Param.ch_status.Charge_stage!=0x03)||((BMS_Param.ch_status.lastPreBalancing+15L*1000L)>HAL_GetTick())){
						if(cellVoltage[j]<BMS_Param.Umin){
							BMS_Param.Umin=cellVoltage[j];
              BMS_Param.CUmin=j+1;
						}
					}
          
//end of part master

// here I made change in jmaster
        if((BMS_Param.BP.TCount[j/8]&(1<<(j%8)))==(1<<(j%8))){ !!!!!!!!!!!!!!!!!!!!
          
          if(auxADC[j]>-60){ $$$$$$$$$$$$$$$$$$$$
            ctsr++; OOOOOOOOOOOOOOOOOOOOOOO\\
// end of change in master 
            
            tsr=tsr+auxADC[j];
            if(auxADC[j]>BMS_Param.Tmax){
              BMS_Param.Tmax=auxADC[j];
              BMS_Param.CTmax=j+1;
            }
            if(auxADC[j]<BMS_Param.Tmin){
              BMS_Param.Tmin=auxADC[j];
              BMS_Param.CTmin=j+1;
            }
          }
				}
			
		}
    if(BMS_Param.Tmin==500)BMS_Param.Tmin=0;
		if(ctsr){ BMS_Param.Tsr=tsr/ctsr; }
		BMS_Param.Ut=uv;
}


/// Процедура выбора нужных ячеек для балансировки
//      =========================   for passive balance =========================
void ProcessBalancing(void)
{
	unsigned char end=0;
	if(BMS_Param.ch_status.Charge_stage==0x02){
		if((BMS_Param.ch_status.lastPreBalancing+15L*1000L)<HAL_GetTick()){
      for(int i=0;i<BMS_Param.BP.CellCount;i++){
        if((BMS_Param.balance.active_balance&0x01)==0){
            if((cellVoltage[i])>(BMS_Param.UFB+BMS_Param.balance.mvCh[(BMS_Param.ch_status.MyChargeStage)-1]/10)){
              BMS_Param.ch_status.CellBalancing[i/8]|=(1<<(i%8));
            }else 
              BMS_Param.ch_status.CellBalancing[i/8]&=~(1<<(i%8));
        }else{
            if((cellVoltage[i])<(BMS_Param.UFB+BMS_Param.balance.mvCh[(BMS_Param.ch_status.MyChargeStage)-1]/10)){
              BMS_Param.ch_status.CellBalancing[i/8]|=(1<<(i%8));
            }else 
              BMS_Param.ch_status.CellBalancing[i/8]&=~(1<<(i%8));
        }
      }
      BMS_Param.ch_status.lastBalancing=HAL_GetTick();
			BMS_Param.ch_status.lastPreBalancing=HAL_GetTick();
		}
	}
    if(BMS_Param.ch_status.Charge_stage==0x03){
      end=1;
      if((BMS_Param.ch_status.lastBalancing+60L*1000L)<HAL_GetTick()){
        end=0;
        if((BMS_Param.ch_status.MyChargeStage<5)||((BMS_Param.balance.bal_cmd&0x03))){
          for(int i=0;i<BMS_Param.BP.CellCount;i++){
            if((BMS_Param.balance.active_balance&0x01)==0){
              if((cellVoltage[i])>(BMS_Param.UFB+BMS_Param.balance.mvBal[(BMS_Param.ch_status.MyChargeStage)-1]/10)){
                BMS_Param.ch_status.CellBalancing[i/8]|=(1<<(i%8));
                end=1;
              }else 
                BMS_Param.ch_status.CellBalancing[i/8]&=~(1<<(i%8));
            }else{
              if((cellVoltage[i])<(BMS_Param.UFB+BMS_Param.balance.mvBal[(BMS_Param.ch_status.MyChargeStage)-1]/10)){
                BMS_Param.ch_status.CellBalancing[i/8]|=(1<<(i%8));
                end=1;
              }else 
                BMS_Param.ch_status.CellBalancing[i/8]&=~(1<<(i%8));
            }
          }
        }
        BMS_Param.ch_status.lastBalancing=HAL_GetTick();
        BMS_Param.ch_status.lastPreBalancing=HAL_GetTick();
    }
    if(BMS_Param.Umax>(BMS_Param.UFB+BMS_Param.balance.mvBal[(BMS_Param.ch_status.MyChargeStage)-1])){
         end=1;
    }
    
    if((HAL_GetTick()-BMS_Param.ch_status.last_stage_time)>(BMS_Param.charge.sBal[BMS_Param.ch_status.MyChargeStage-1]*1000L*60L)){
         end=0;
    }
