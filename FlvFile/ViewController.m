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
#include "BIH264Decoder.h"
#import "HJOpenGLView.h"


@interface ViewController ()
{
    NSMutableArray* _mutArray;
}

@property (nonatomic, strong) HJOpenGLView  *playView;

@property (weak, nonatomic) IBOutlet UIButton* bt;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    // playView
    [self.view addSubview:self.playView];
    self.playView.frame = CGRectMake(PLAYVIEW_PORTRAIT_STARTX, PLAYVIEW_PORTRAIT_STARTY, PLAYVIEW_PORTRAIT_WIDTH, PLAYVIEW_PORTRAIT_HEIGHT);
    [self.playView setupGL];
    [self.view bringSubviewToFront:self.bt];
    
    _mutArray = [NSMutableArray array];
    
    double store22 = CFSwapInt64BigToHost(1.5f);
    NSLog(@"%f",store22);
}

- (IBAction)btClicked:(id)sender {
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
    
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
        [self processTag:flvTag];
        freeFlvTag(flvTag);
        flvTag = NULL;
        flvTag = readFlvTag(&buffer);
    }
        [self displayPic];
        EndVideoToolBox();
        NSLog(@"$$  total_alloc_size:%zu",total_alloc_size);
    });
     
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
                NSMutableData* mutData = [NSMutableData data];
                uint32_t length = 0;
                while (videoData != NULL) {
                    bi_flv_video_NALU_data*  naluData = videoData->nalu_data;
//                    inputPackage->packetBuffer = naluData->data;
//                    inputPackage->packetSize = naluData->length;
                    inputPackage->cts = package->cts;
                    [mutData appendBytes:naluData->data length:naluData->length];
                    length += naluData->length;
                    videoData = videoData->next;
                }
                inputPackage->packetBuffer = (void*)[mutData bytes];
                inputPackage->packetSize = length;
                [self recvVideoTag:flvTag package:inputPackage];
            }
        }
        bi_free(inputPackage);
    }
}

-(void)displayPic{
    

    for (NSData* data in _mutArray) {
        bi_video_output_package package;
        [data getBytes:&package length:sizeof(package)];
        bi_video_output_package* outputPackage = &package;
        
//        uint32_t disPlayTime = outputPackage->timestamp*90+outputPackage->cts;

            [self.playView displayPixelBuffer:outputPackage->bufferRef];
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
}


-(HJOpenGLView *)playView
{
    if (!_playView) {
        _playView = [[HJOpenGLView alloc] init];
    }
    return _playView;
}

@end
