//
//  ViewController.m
//  FlvFile
//
//  Created by kirk on 2018/12/25.
//  Copyright © 2018 kirk. All rights reserved.
//

#import "ViewController.h"
#include "BIFlvDecoder.h"
#include "BIMalloc.h"
#include "BIByteReader.h"
//#import "HJH264Decoder.h"
#include "BIH264Decoder.h"
#import "HJOpenGLView.h"

#define SCREENWIDTH [UIScreen mainScreen].bounds.size.width
#define SCREENHEIGHT [UIScreen mainScreen].bounds.size.height

#define PLAYVIEW_PORTRAIT_STARTX    0
#define PLAYVIEW_PORTRAIT_STARTY    (SCREENHEIGHT - SCREENWIDTH * (480 / 640.0)) / 2.0
#define PLAYVIEW_PORTRAIT_WIDTH     SCREENWIDTH
#define PLAYVIEW_PORTRAIT_HEIGHT    SCREENWIDTH * (480 / 640.0)

@interface ViewController ()
{
    NSFileHandle* _fileHandle;
//    BIFlvDecoder* _decoder;
    NSMutableArray* _mutArray;
}

//@property (nonatomic, retain) HJH264Decoder *decoder;

@property (nonatomic, strong) HJOpenGLView  *playView;

@property (weak, nonatomic) IBOutlet UIButton* bt;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
//    _decoder = [[BIFlvDecoder alloc] init];
    // Do any additional setup after loading the view, typically from a nib.
    
//    data = [_fileHandle readDataOfLength:4];
//    [self readTag];
//    NSMutableData* mutData = [NSMutableData data];
//    int length = 0;//CFSwapInt32BigToHost
//    [mutData appendBytes:&length length:sizeof(length)];
    
    // playView
    [self.view addSubview:self.playView];
    self.playView.frame = CGRectMake(PLAYVIEW_PORTRAIT_STARTX, PLAYVIEW_PORTRAIT_STARTY, PLAYVIEW_PORTRAIT_WIDTH, PLAYVIEW_PORTRAIT_HEIGHT);
    //    NSLog(@"-- %d, %f, %f, %f",PLAYVIEW_PORTRAIT_STARTX, PLAYVIEW_PORTRAIT_STARTY, PLAYVIEW_PORTRAIT_WIDTH, PLAYVIEW_PORTRAIT_HEIGHT);
    [self.playView setupGL];
    [self.view bringSubviewToFront:self.bt];
    
    _mutArray = [NSMutableArray array];
    
    double store22 = CFSwapInt64BigToHost(1.5f);
    NSLog(@"%f",store22);
}

- (IBAction)btClicked:(id)sender {
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
    
    NSString* filePath = [[NSBundle mainBundle] pathForResource:@"cuc_ieschool" ofType:@"flv"];
    _fileHandle = [NSFileHandle fileHandleForReadingAtPath:filePath];
    NSData* data = [_fileHandle readDataOfLength:BI_READ_LENGTH*300];
    Byte *testByte = (Byte *)[data bytes];
    
    bi_data_buffer buffer;
    buffer.data = testByte;
    buffer.length = BI_READ_LENGTH*300;
    buffer.cursor = 0;
    
    bi_flv_header* header = readHeaderWithAlloc(&buffer);
//    NSLog(@"header length:%d ## %zu",header->header_size,total_alloc_size);
    freeFlvHeader(header);
    header = NULL;
//    NSLog(@"total_alloc_size:%zu",total_alloc_size);
//    bi_flv_tag* tag = readFlvTag(&buffer);
////    NSLog(@"total_alloc_size:%zu",total_alloc_size);
//    bi_script_object_properties_list_node* node = tag->script->value->array_value->variables_list;
//    while (node != NULL) {
//        bi_script_data_object_property_value* value = node->property_value;
//        NSLog(@"%s",value->property_name->string_data);
//        node = node->next;
//    }
//    freeFlvTag(tag);
//    NSLog(@"total_alloc_size:%zu",total_alloc_size);
    bi_flv_tag* flvTag = readFlvTag(&buffer);
    while (flvTag != NULL) {
//        NSLog(@"^^^^ total_alloc_size:%zu",total_alloc_size);
        [self processTag:flvTag];
        freeFlvTag(flvTag);
//        NSLog(@"**  total_alloc_size:%zu",total_alloc_size);
        flvTag = NULL;
        flvTag = readFlvTag(&buffer);
    }
        [self displayPic];
        NSLog(@"$$  total_alloc_size:%zu",total_alloc_size);
    });
//    bi_flv_tag* spsTag = readFlvTag(&buffer);
//        freeFlvTag(spsTag);
//    bi_flv_tag* audioTag = readFlvTag(&buffer);
//        freeFlvTag(audioTag);
//    bi_flv_tag* videoTag = readFlvTag(&buffer);
//        freeFlvTag(videoTag);
//        });
}

-(void)processTag:(bi_flv_tag*)flvTag{
    if (flvTag == NULL) {
        return;
    }
    if (flvTag->type == flv_tag_type_video) {
        bi_video_input_package* inputPackage = (bi_video_input_package*)bi_malloc(sizeof(bi_video_input_package));
        inputPackage->timestamp = flvTag->timestamp;
        inputPackage->timestamp_extends = flvTag->timestamp_extends;
        inputPackage->cts = 0;
        inputPackage->type = flvTag->video->type;
        
        bi_flv_video_avc_package_data* package = flvTag->video->avc_package;
        if (package != NULL){
            if (package->avc_type == video_avc_sequenceheader) {
                bi_avc_decoder_configuration_record* record = package->configuration_record;
                
                bi_flv_video_SPS_PPS_list_node* spsList = record->sps_list;
                
                while (spsList != NULL) {
                    bi_flv_video_SPS_or_PPS_data* spsData = spsList->sps_or_pps_data;
                    inputPackage->packetBuffer = spsData->data;
                    inputPackage->packetSize = spsData->length;
                    inputPackage->cts = package->cts;
                    [self recvVideoTag:flvTag package:inputPackage];
                    spsList = spsList->next;
                }
                
                bi_flv_video_SPS_PPS_list_node* ppsList = record->pps_list;
                
                while (ppsList != NULL) {
                    bi_flv_video_SPS_or_PPS_data* ppsData = ppsList->sps_or_pps_data;
                    inputPackage->packetBuffer = ppsData->data;
                    inputPackage->packetSize = ppsData->length;
                    inputPackage->cts = package->cts;
                    [self recvVideoTag:flvTag package:inputPackage];
                    ppsList = ppsList->next;
                }
                
            }else{
                bi_flv_video_NALU_list_node* videoData = package->video_data;
                while (videoData != NULL) {
                    bi_flv_video_NALU_data*  naluData = videoData->nalu_data;
                    inputPackage->packetBuffer = naluData->data;
                    inputPackage->packetSize = naluData->length;
                    inputPackage->cts = package->cts;
                    [self recvVideoTag:flvTag package:inputPackage];
                    videoData = videoData->next;
                }
            }
        }
        bi_free(inputPackage);
    }
}

-(void)displayPic{
    
    [_mutArray sortUsingComparator:^NSComparisonResult(NSData*  _Nonnull obj1, NSData*  _Nonnull obj2) {
        bi_video_output_package package1;
        [obj1 getBytes:&package1 length:sizeof(package1)];
        bi_video_output_package* outputPackage1 = &package1;
        uint32_t disPlayTime1 = outputPackage1->timestamp*90+outputPackage1->cts;
        
        bi_video_output_package package2;
        [obj2 getBytes:&package2 length:sizeof(package2)];
        bi_video_output_package* outputPackage2 = &package2;
        uint32_t disPlayTime2 = outputPackage2->timestamp*90+outputPackage2->cts;
        
        if (disPlayTime1<disPlayTime2) {
            return NSOrderedAscending;
        }else if(disPlayTime1>disPlayTime2){
            return NSOrderedDescending;
        }
        return NSOrderedSame;
    }];
    
    uint32_t time = 0;
    for (NSData* data in _mutArray) {
        bi_video_output_package package;
        [data getBytes:&package length:sizeof(package)];
        bi_video_output_package* outputPackage = &package;//(bi_video_output_package *)[data bytes];
        
        uint32_t disPlayTime = outputPackage->timestamp*90+outputPackage->cts;
//        NSLog(@"usleep:%d  ## %d ## %d # %d",disPlayTime-time,outputPackage->timestamp*90,outputPackage->cts,outputPackage->type);
        usleep((disPlayTime-time)*1);
        time=disPlayTime;
        
//        if (outputPackage->cts==0) {
        NSLog(@"122345");
            [self.playView displayPixelBuffer:outputPackage->bufferRef];
        NSLog(@"6786786");
//        }
        
        CVPixelBufferRelease(outputPackage->bufferRef);
        
    }
    [_mutArray removeAllObjects];
}


-(void)recvVideoTag:(bi_flv_tag*)flvTag package:(bi_video_input_package*)inputPackage
{
    bi_video_output_package* outputPackage = decoderInputTag(inputPackage);
    if (outputPackage != NULL) {
//        [self.playView displayPixelBuffer:outputPackage->bufferRef];
//        CVPixelBufferRelease(outputPackage->bufferRef);
//        bi_free(outputPackage);
        NSData* data = [NSData dataWithBytes:outputPackage length:sizeof(bi_video_output_package)];
        [_mutArray addObject:data];
        bi_free(outputPackage);
    }
    //    _speedLength = length;
//    printf("----- recved len = %d \n", length);
    
    // 解码
//    [self.decoder startH264DecodeWithVideoData:(char *)videoData andLength:length+4 andReturnDecodedData:^(CVPixelBufferRef pixelBuffer) {
//
//        // OpenGL渲染
//
//            [self.playView displayPixelBuffer:pixelBuffer];
//
//    }];
}

//-(HJH264Decoder *)decoder
//{
//    if (!_decoder) {
//        _decoder = [[HJH264Decoder alloc] init];
//    }
//    return _decoder;
//}

-(HJOpenGLView *)playView
{
    if (!_playView) {
        _playView = [[HJOpenGLView alloc] init];
    }
    return _playView;
}

+ (NSString *)convertDataToHexStr:(NSData *)data
{
    if (!data || [data length] == 0) {
        return @"";
    }
    NSMutableString *string = [[NSMutableString alloc] initWithCapacity:[data length]];
    
    [data enumerateByteRangesUsingBlock:^(const void *bytes, NSRange byteRange, BOOL *stop) {
        unsigned char *dataBytes = (unsigned char*)bytes;
        for (NSInteger i = 0; i < byteRange.length; i++) {
            NSString *hexStr = [NSString stringWithFormat:@"%x", (dataBytes[i]) & 0xff];
            if ([hexStr length] == 2) {
                [string appendString:hexStr];
            } else {
                [string appendFormat:@"0%@", hexStr];
            }
        }
    }];
    
    return string;
}

@end
