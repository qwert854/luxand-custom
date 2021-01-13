#include <iostream>
#include <stdio.h>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <strings.h>
#include <stdlib.h>
#include "resource.h"
#include "LuxandFaceSDK.h"
#include <vector>
#include <string.h>
#include <thread>
#include <time.h>


// 각폴더안디렉토리추출.
typedef std::vector < std::string > FILE_LIST;
class CFileList
        {
public : CFileList();
    ~ CFileList();
        public : bool GetFileList(FILE_LIST & list, std::string strDir); // 디렉토리 및 파일목록을 구한다
};
CFileList::CFileList() {}
CFileList:: ~ CFileList() {}
bool CFileList::GetFileList(FILE_LIST & list, std::string strDir)
{
    struct stat st;
    memset(& st, 0, sizeof(st));
    lstat(strDir.c_str(), & st);
    if (!S_ISDIR(st.st_mode))
    {
        std::cout << strDir + " is not directory" << std::endl;
        return false;
    }
    DIR * dir;
    struct dirent * ent;
    if ((dir = opendir(strDir.c_str())) == NULL)
    {
        std::cout << strDir + " open error" << std::endl;
        return false;
    }
    while ((ent = readdir(dir)) != NULL)
    {
        memset(& st, 0, sizeof(st));
        std::string strFilePath = strDir + "/" + ent -> d_name;
        while (strFilePath.find("//") != std::string::npos)
            strFilePath.replace(strFilePath.find("//"), 2, "/");

        lstat(strFilePath.c_str(), & st);
        if (S_ISDIR(st.st_mode))// 디렉토리 이면
        {
            if (strcmp(ent -> d_name, ".") == 0 || strcmp(ent -> d_name, "..") == 0)
                continue;
            std::string strSubDir = strDir + "/" + ent -> d_name;
            GetFileList(list, strSubDir); // 디렉토리 안으로 들어감
        }
        else
            {
            list.push_back(strFilePath); // 파일경로를 넣음
        }
    }
// 읽은 디렉토리 닫기
    closedir(dir);
    return true;
}
// THREAD
void compareFunction(FILE_LIST s_file_list, FILE_LIST data_list, int thread_number) {
    FILE_LIST::iterator itr;
    FILE_LIST::iterator itr2;
    HImage imageHandle;
    HImage imageHandle2;
    // 같은이름 파일 비교
    for (itr = s_file_list.begin(); itr != s_file_list.end(); itr ++)
    {
        std::string s1 = strchr(itr -> c_str() + 18, '/');//문자열검사(글자 수를 센 후 18번째 글자 뒤에오는 /까지 읽는다)
        // 파일명.png
        char temp[100];
        char * filename;
        strcpy(temp, s1.c_str()); //s1의 문자열을 복사한다.
        filename = strtok(temp, "/");//문자열을 /까지의 문자열을 분리한다.
        // tok= 파일명.png
        filename = strtok(filename, ".");//파일명뒤에오는.png앞까지 분리한다.
        // filename=파일명
        for (itr2 = data_list.begin(); itr2 != data_list.end(); itr2 ++)
        {
            // cout<<thread_number<<"thread working\n";
            std::string s2 = strchr(itr2 -> c_str() + 18, '/');   // s2= /폴더명/파일명.png
            char temp2[100];
            char * foldername;
            strcpy(temp2, s2.c_str());
            char * tok = strtok(temp2, "/");//폴더명과 파일명사이의 /를 분리한다.
                                                 // tok = 폴더명/파일명.png
            foldername = tok; //foldername=폴더명

            if (strcmp(filename, foldername) == 0) //파일명과 폴더명의 문자열을 비교한다 ==0 파일명과 폴더명이 일치할때
            {
                FSDK_LoadImageFromFile(& imageHandle, itr -> c_str());//이미지불러오기.
                FSDK_LoadImageFromFile(& imageHandle2, itr2 -> c_str());
                FSDK_FaceTemplate template1,template2; //template 선언.

                float MatchingThreshold,Similarity;
                FSDK_GetMatchingThresholdAtFAR(0.02, & MatchingThreshold);
                FSDK_GetFaceTemplate(imageHandle, & template1);
                FSDK_GetFaceTemplate(imageHandle2, & template2);
                FSDK_MatchFaces(& template1, & template2, & Similarity);
                if (Similarity == 1) {
                    std::cout << "filename:" << filename << "\n";
                    std::cout << "folder:" << foldername << "\n";
                    printf("%.3f \n", Similarity);

                }
            }
        }
    }
}
int main()
{
    struct timespec begin,end;
    clock_gettime(CLOCK_MONOTONIC, &begin);
    if (FSDKE_OK != FSDK_ActivateLibrary("HMGN4CkTERu0sktE6QbCrt/MS68qKuT5LQy35r6RbMNh1eBowWlj+RY9fHbg2jsuvH06uQdo4SkRDFHn0sU7KAzb0EevK/+l8qClcJZZIMUf2dP+FYwxlXdYFLr13oR95GwU5GHv+pPC0tVkOBbM0FMxO4Hn6gHnM3v5+gAOkDQ=")) {

        printf("Please run the License Key Wizard (Start - Luxand - FaceSDK - License Key Wizard)\\n");
        printf("Error activating FaceSDK");
        exit(-1);
    }
    FSDK_Initialize("");
    FSDK_InitializeCapturing();

    // 각 폴더안 디렉토리
    CFileList FileList;
    FILE_LIST file_list;
    FILE_LIST data_list;
    FILE_LIST temp_list;
    FileList.GetFileList(file_list, "/home/sw/Desktop/wow/");  // 비교할이미지
    FileList.GetFileList(data_list, "/home/sw/Desktop/img/");  // 이미지저장

    int listSize = file_list.size();
    int totalSize = listSize;
    std::vector < std::thread > t_vector;
    int file_num = 0;
    int thread_num = 0;
    int SEGMENTATION_SIZE=totalSize/4;  // 총 파일 수 에서 4를 나누어 스레드가 4~5개 실행되게함
    if (listSize > SEGMENTATION_SIZE) //list개수가 더 많을 때
    {
        while (listSize > 0)
        {
            temp_list.clear();
            if (listSize - SEGMENTATION_SIZE > 0)
            {
                for (int j = file_num; j < file_num + SEGMENTATION_SIZE; ++j)
                {
                    temp_list.push_back(file_list[j]);
                    listSize = listSize - 1;
                }
                file_num = file_num + SEGMENTATION_SIZE;
                t_vector.push_back(std::thread(compareFunction, temp_list, data_list, thread_num ++));
            }
            else //나머지 실행.
                {
                for (int j = file_num; j < totalSize; ++j)
                {
                    temp_list.push_back(file_list[j]);
                    listSize = listSize - 1;
                }
                t_vector.push_back(std::thread(compareFunction, temp_list, data_list, thread_num ++));
            }
        }
    }
    else{t_vector.push_back(std::thread(compareFunction, file_list, data_list, thread_num ++));//파일수가 더 적을때
    }
    std::vector < std::thread >:: iterator itr;
    for (itr = t_vector.begin(); itr != t_vector.end(); itr ++){
        itr -> join();
    }
    clock_gettime(CLOCK_MONOTONIC, & end);
    std::cout << (end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec) / 1000000000.0 << std::endl;
    return 0;
}