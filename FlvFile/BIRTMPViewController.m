//
//  BIRTMPViewController.m
//  FlvFile
//  推流 播放
//  Created by kirk on 2019/1/14.
//  Copyright © 2019 kirk. All rights reserved.
//

#import "BIRTMPViewController.h"
#import "rtmp.h"
#import "log.h"
#include "BIFlvDecoder.h"
#include "BIByteReader.h"

@interface BIRTMPViewController ()
{
    RTMP *_rtmp;
}
@property (weak, nonatomic) IBOutlet UIButton* bt;
@end

@implementation BIRTMPViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}

-(void)connect{
    //1 分配 RTMP 对象，并初始化
    _rtmp = RTMP_Alloc();
    RTMP_Init(_rtmp);
    //2 设置 rtmp URL
    int status = RTMP_SetupURL(_rtmp,"rtmp://localhost:1935/live/room");
    NSLog(@"%d",status);
    RTMP_EnableWrite(_rtmp);
    //3 设置缓冲区大小
    RTMP_SetBufferMS(_rtmp, 3600*1000);
    //4 连接流媒体服务器
    status = RTMP_Connect(_rtmp,NULL);
    NSLog(@"%d",status);
    status = RTMP_ConnectStream(_rtmp,0);
    NSLog(@"%d",status);
    
    
}

-(void)close{
    //关闭连接
    RTMP_Close(_rtmp);
    RTMP_Free(_rtmp);
}

- (IBAction)btClicked:(id)sender {
    __weak typeof(self) weakSelf = self;
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
//        publish_using_write();
        
        [weakSelf connect];
        NSString* filePath = [[NSBundle mainBundle] pathForResource:@"cuc_ieschool" ofType:@"flv"];
        NSFileHandle* _fileHandle = [NSFileHandle fileHandleForReadingAtPath:filePath];
        NSData* data = [_fileHandle readDataOfLength:BI_READ_LENGTH*3000];
        Byte *testByte = (Byte *)[data bytes];
        
        bi_data_buffer buffer;
        buffer.data = testByte;
        buffer.length = BI_READ_LENGTH*3000;
        buffer.cursor = 0;
        
        bi_flv_header* header = readHeaderWithAlloc(&buffer);
        freeFlvHeader(header);
        header = NULL;
        buffer.cursor +=4;
        
        bi_rtmp_flv_tag* flvTag = readRTMPFlvTag(&buffer);
        while (flvTag != NULL) {
            [weakSelf processTag:flvTag];
            freeRTMPFlvTag(flvTag);
            flvTag = NULL;
            flvTag = readRTMPFlvTag(&buffer);
        }
//        NSLog(@"$$  total_alloc_size:%zu",total_alloc_size);
        [weakSelf close];
        
    });
    
}

-(void)processTag:(bi_rtmp_flv_tag*)flvTag{
    if (flvTag == NULL /*&& flvTag->type != flv_tag_type_video*/) {
        return;
    }
//    sleep(1);
    int write_ret = RTMP_Write(_rtmp, flvTag->data, flvTag->data_size+11+4);
    NSLog(@"write_ret = %d",write_ret);
    
}

/*  以下是  参考博客
    ---------------------
    作者：雷霄骅
    来源：CSDN
    原文：https://blog.csdn.net/leixiaohua1020/article/details/42104945
    版权声明：本文为博主原创文章，转载请附上博文链接！
 */

#define HTON16(x)  ((x>>8&0xff)|(x<<8&0xff00))
#define HTON24(x)  ((x>>16&0xff)|(x<<16&0xff0000)|(x&0xff00))
#define HTON32(x)  ((x>>24&0xff)|(x>>8&0xff00)|\
(x<<8&0xff0000)|(x<<24&0xff000000))
#define HTONTIME(x) ((x>>16&0xff)|(x<<16&0xff0000)|(x&0xff00)|(x&0xff000000))

int ReadU8(uint32_t *u8,FILE*fp){
    if(fread(u8,1,1,fp)!=1)
        return 0;
    return 1;
}

int ReadU24(uint32_t *u24,FILE*fp){
    if(fread(u24,3,1,fp)!=1)
        return 0;
    *u24=HTON24(*u24);
    return 1;
}

int PeekU8(uint32_t *u8,FILE*fp){
    if(fread(u8,1,1,fp)!=1)
        return 0;
    fseek(fp,-1,SEEK_CUR);
    return 1;
}

int ReadU32(uint32_t *u32,FILE*fp){
    if(fread(u32,4,1,fp)!=1)
        return 0;
    *u32=HTON32(*u32);
    return 1;
}

int ReadTime(uint32_t *utime,FILE*fp){
    if(fread(utime,4,1,fp)!=1)
        return 0;
    *utime=HTONTIME(*utime);
    return 1;
}

int publish_using_write(){
    uint32_t start_time=0;
    uint32_t now_time=0;
    uint32_t pre_frame_time=0;
    uint32_t lasttime=0;
    int bNextIsKey=0;
    char* pFileBuf=NULL;
    
    //read from tag header
    uint32_t type=0;
    uint32_t datalength=0;
    uint32_t timestamp=0;
    
    RTMP *rtmp=NULL;
    
    FILE*fp=NULL;
    NSString* filePath = [[NSBundle mainBundle] pathForResource:@"cuc_ieschool" ofType:@"flv"];
    
    const char* url =  [filePath cStringUsingEncoding:(NSUTF8StringEncoding)];
    fp=fopen(url,"r");
    if (!fp){
        return -1;
    }
    
    /* set log level */
    //RTMP_LogLevel loglvl=RTMP_LOGDEBUG;
    //RTMP_LogSetLevel(loglvl);
    
    
    rtmp=RTMP_Alloc();
    RTMP_Init(rtmp);
    //set connection timeout,default 30s
    rtmp->Link.timeout=5;
    if(!RTMP_SetupURL(rtmp,"rtmp://localhost:1935/live/room"))
    {
        RTMP_Free(rtmp);
        return -1;
    }
    
    RTMP_EnableWrite(rtmp);
    //1hour
    RTMP_SetBufferMS(rtmp, 3600*1000);
    if (!RTMP_Connect(rtmp,NULL)){
        RTMP_Free(rtmp);
        return -1;
    }
    
    if (!RTMP_ConnectStream(rtmp,0)){
        RTMP_Log(RTMP_LOGERROR,"ConnectStream Err\n");
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
        return -1;
    }
    
    printf("Start to send data ...\n");
    
    //jump over FLV Header
    fseek(fp,9,SEEK_SET);
    //jump over previousTagSizen
    fseek(fp,4,SEEK_CUR);
    start_time=RTMP_GetTime();
    while(1)
    {
        if((((now_time=RTMP_GetTime())-start_time)
            <(pre_frame_time)) && bNextIsKey){
            //wait for 1 sec if the send process is too fast
            //this mechanism is not very good,need some improvement
            if(pre_frame_time>lasttime){
                RTMP_LogPrintf("TimeStamp:%8u ms\n",pre_frame_time);
                lasttime=pre_frame_time;
            }
            usleep(1000);
            continue;
        }
        
        //jump over type
        fseek(fp,1,SEEK_CUR);
        if(!ReadU24(&datalength,fp))
            break;
        
        
        if(!ReadTime(&timestamp,fp))
            break;
        //jump back
        fseek(fp,-8,SEEK_CUR);
        
        pFileBuf=(char*)malloc(11+datalength+4);
        memset(pFileBuf,0,11+datalength+4);
        if(fread(pFileBuf,1,11+datalength+4,fp)!=(11+datalength+4))
            break;
        
        pre_frame_time=timestamp;
        
        if (!RTMP_IsConnected(rtmp)){
            RTMP_Log(RTMP_LOGERROR,"rtmp is not connect\n");
            break;
        }
        if (!RTMP_Write(rtmp,pFileBuf,11+datalength+4)){
            RTMP_Log(RTMP_LOGERROR,"Rtmp Write Error\n");
            break;
        }
        
        free(pFileBuf);
        pFileBuf=NULL;
        
        if(!PeekU8(&type,fp))
            break;
        if(type==0x09){
            if(fseek(fp,11,SEEK_CUR)!=0)
                break;
            if(!PeekU8(&type,fp)){
                break;
            }
            if(type==0x17)
                bNextIsKey=1;
            else
                bNextIsKey=0;
            fseek(fp,-11,SEEK_CUR);
        }
    }
    
    RTMP_LogPrintf("\nSend Data Over\n");
    
    if(fp)
        fclose(fp);
    
    if (rtmp!=NULL){
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
        rtmp=NULL;
    }
    
    if(pFileBuf){
        free(pFileBuf);
        pFileBuf=NULL;
    }
    
    return 0;
}

@end
