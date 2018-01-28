#include "public.h"
#include "hardware/IO.h"
#include "function/function.h"
#include "opencv2/highgui.hpp"
#include "opencv2/opencv.hpp"
using namespace cv;

int main()
{
    /******************************************
    Opencv part
    *******************************************
    cout << "Hello World!" << endl;

    VideoCapture USBvideo;
    USBvideo.open("/dev/video2");
    USBvideo.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    USBvideo.set(CV_CAP_PROP_FRAME_HEIGHT, 720);//设置分辨率
    printf("video fps is :%lf\n",USBvideo.get(CV_CAP_PROP_FPS));
    Mat frame;
    USBvideo >> frame; // get a new frame from camera
    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(95);//这就是质量 默认值是95
    imwrite("small_image.jpeg",frame,compression_params);//写入图片*/

    /******************************************
    Serial part & NET_C
    *******************************************
    printf("Please input the addr or path\n");
    char path[20];
    scanf("%s",path);
    Pack_Processor *client = new Pack_Processor(COMM_NET_C,path,8000);
    printf("Pack_Processor ok\n");
    pack_buff tmp_buff;
    char *i = (char *)"A";
    tmp_buff.buff = i;
    tmp_buff.length = 2;
    tmp_buff.sour_ID = LOCAL_ID;
    tmp_buff.dest_ID = 2;
    tmp_buff.type = SEND_USER_OPERATION;
    client->send_data(tmp_buff);
    //free(tmp_buff.buff);
    delete(client);
    printf("quit\n");*/

    Pack_Processor *server = new Pack_Processor(COMM_NET_S,NULL,8000);
    printf("Pack_Processor ok\n");
    pack_buff tmp_buff;
    loop:usleep(100000);
    server->get_pack(SEND_USER_OPERATION,ref(tmp_buff));
    if(tmp_buff.buff == NULL) goto loop;
    printf("have get new pack, \ndata is:%c\nlength is:%d\n",*tmp_buff.buff,tmp_buff.length);
    free(tmp_buff.buff);//this is so important!
    delete(server);

}
