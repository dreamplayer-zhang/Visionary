#if 0
#include "stdafx.h"
#include "watershed.h"

#include <map>
#include <set>
#include <list>
#include "math.h"
using namespace std;

namespace watershed
{

//ÿ��עˮ���
#define STEP_ASH 10
//����������δ����û������
#define REGION_NO_DEFAULT -1
//�����޶���С����Ĵ�С
#define MIN_REGIONPIXEL_COUNT 1
//�ݲ�,�������Ʒָ�ľ�ȷ��
#define TOLERANCE 28

//�궨��,Ϊ���������ı���
#define FOR_EACH(pContainer, it)            \
    for ((it) = (pContainer)->begin(); (it) != (pContainer)->end(); (it)++)

//��������
typedef unsigned char BYTE;
typedef BYTE *ByteImage;
typedef long RegionNo;

//�����㷨�е�������
typedef struct _WatershedPixel
{
    int nX;              //��������
    int nY;
    BYTE btAsh;          //���ػҶ�
    bool bDrown;         //�Ƿ��Ѿ�����û
    RegionNo lRegionNo;  //��������ı��
} WatershedPixel;

//�����㷨�б��ָ����һ������, ����ˮ��
typedef struct _WatershedRegion
{
    RegionNo regionNo;       //������
    BYTE btLowest;           //��ˮ������ĻҶ�
    WatershedPixel *pPixel;  //��������Ϊ���ӵ���������
    long lPixelCount;        //����Ĵ�С
    long lTotalAsh;
}WatershedRegion;

//�����㷨�е�ͼ��
typedef struct _WatershedImage
{
    BYTE bTolerance;         //�ݲ�
    int nWidth;              //ͼ��ߴ�
    int nHeight;
    WatershedPixel *pixels;  //���ؼ�
    map<RegionNo, WatershedRegion *> regionMap;  //���ڼ�¼������Ϣ
}WatershedImage;

/****************************************************************************
* ͨ������������
* ������pWatershedImage ͼ��
*       nX, nY ��������
****************************************************************************/
inline WatershedPixel *GetPixelXY(
    WatershedImage *pWatershedImage, 
    int nX, int nY
)
{
    long lOffset = nY * pWatershedImage->nWidth + nX;
    return &(pWatershedImage->pixels[lOffset]);
}

/****************************************************************************
* ����һ��Ψһ��������
****************************************************************************/
inline RegionNo CreateRegionNo()
{
    static RegionNo lRegionNo = 0;
    lRegionNo += 1;
    return lRegionNo;
}

/****************************************************************************
* �������ĺϷ���
* ������pWatershedImage ͼ��
*       nX, nY ��������
****************************************************************************/
inline bool CheckXY(WatershedImage *pWatershedImage, int nX, int nY)
{
    return (nX >= 0 && nX < pWatershedImage->nWidth && 
        nY >= 0 && nY < pWatershedImage->nHeight);
}

/****************************************************************************
* ��ʼ��ͼ��
* ������pWatershedImage ��ˮ��ͼ��
*       imageIn ����ĵ���ͼ��
*       w, h ͼ��ߴ�
*       btTolerance �ݲ�
****************************************************************************/
bool InitWatershedImage(
    WatershedImage *pWatershedImage, 
    ByteImage imageIn, 
    unsigned int w, unsigned int h,
    BYTE btTolerance
)
{
    bool bRetn = false;
    long lIndex, lPixelOffset;
    int nTotalAsh;
    int nAsh;
    WatershedPixel *pPixels = NULL;
    WatershedPixel *pCurrentPixel;

    //�������ؼ�
    pPixels = new WatershedPixel[w * h];
    if (pPixels == NULL)
        goto Exit0;

    //�������
    pWatershedImage->nHeight = h;
    pWatershedImage->nWidth = w;
    pWatershedImage->pixels = pPixels; 
    pWatershedImage->bTolerance = btTolerance;
    pWatershedImage->regionMap.clear();

    //��ʼ����ˮ��ͼ��
    for (lIndex = 0; lIndex < (long)(w * h); lIndex++)
    {
        lPixelOffset = lIndex * 4;
        nTotalAsh = imageIn[lPixelOffset] 
        + imageIn[lPixelOffset + 1]
        + imageIn[lPixelOffset + 2];
        pCurrentPixel = &(pPixels[lIndex]);
        pCurrentPixel->nX = lIndex % w;
        pCurrentPixel->nY = lIndex / w;

        //����ƽ���Ҷȣ�����ʽ��ΪSTEP_ASH��������
        nAsh = nTotalAsh / 3;
        nAsh = nAsh/ STEP_ASH * STEP_ASH;

        pCurrentPixel->btAsh = nAsh;
        pCurrentPixel->bDrown = false;
        pCurrentPixel->lRegionNo = REGION_NO_DEFAULT;
    }

    bRetn = true;
Exit0:
    return bRetn;
}

/****************************************************************************
* �ͷ�ͼ��
* ������pWatershedImage ��ˮ��ͼ��
****************************************************************************/
bool FinalizeWatershedImage(
    WatershedImage *pWatershedImage
)
{
    bool bRetn = false;
    map<RegionNo, WatershedRegion *>::iterator itPair;
    WatershedRegion *pRegion;

    if (NULL == pWatershedImage)
        goto Exit0;

    FOR_EACH(&(pWatershedImage->regionMap), itPair)
    {
        pRegion = (*itPair).second;
        delete pRegion;
    }

    pWatershedImage->regionMap.clear();
    delete[] pWatershedImage->pixels;
    bRetn = true;
Exit0:
    return bRetn;
}

/****************************************************************************
* �����ؼ��ϲ���ָ������
* ������pPixelList ���ؼ�
*       pRegion    Ŀ������
****************************************************************************/
bool AddPixelsToRegion(
    list<WatershedPixel *> *pPixelList, 
    WatershedRegion *pRegion
)
{
    list<WatershedPixel *>::iterator itPix;
    WatershedPixel *pPixel = NULL;
    RegionNo regionNo;
    long lTotalAsh;

    regionNo = pRegion->regionNo;
    lTotalAsh = pRegion->lTotalAsh;

    FOR_EACH(pPixelList, itPix)
    {
        pPixel = *itPix;
        pPixel->lRegionNo = regionNo;
        lTotalAsh += pPixel->btAsh;
    }
    pRegion->lPixelCount += (long)pPixelList->size();
    pRegion->lTotalAsh = lTotalAsh;

    return true;
}


/****************************************************************************
* �������㷨������ͬ�Ҷȵ�δ��û����
* ������pSeedPixel    ��������
*       pNeighbors    ���������з��ֵ���������
*       pPixelsFound  ������������
*       *pTmpRegionNo Ϊ��������������ʱ����������
****************************************************************************/
bool SeedSearchPixel(
    WatershedImage *pWatershedImage,
    WatershedPixel *pSeedPixel,
    set<RegionNo> *pNeighbors,
    list<WatershedPixel *> *pPixelsFound,
    RegionNo *pTmpRegionNo
)
{
    //�����
    static int dx[8] = {-1, 0, 1, 1, 1, 0, -1, -1};
    static int dy[8] = {-1, -1, -1, 0, 1, 1, 1, 0};

    //�����㷨��Ҫ����ʱ����
    static list<WatershedPixel *> list1, list2;

    RegionNo newRegionNo;
    list<WatershedPixel *>::iterator itPix;
    list<WatershedPixel *> *pCurrentList, *pNewList, *pSwapList;
    WatershedPixel *pNewPixel, *pPixel;
    WatershedRegion *pNewRegion = NULL;
    BYTE btAsh;

    int nIndex, nNx, nNy;
    long lPixelCount = 0;
    long lNeighborCount = 0;

    pCurrentList = &list1;
    pNewList = &list2;

    btAsh = pSeedPixel->btAsh;
    newRegionNo = CreateRegionNo();
    *pTmpRegionNo = newRegionNo;
    pCurrentList->clear();
    pCurrentList->push_back(pSeedPixel);
    pPixelsFound->push_back(pSeedPixel);
    pSeedPixel->lRegionNo = newRegionNo;
    pSeedPixel->bDrown = true;

    while (pCurrentList->size() > 0)
    {
        pNewList->clear();
        FOR_EACH(pCurrentList, itPix)
        {
            pPixel = *itPix;
            for (nIndex = 0; nIndex < 8; nIndex++)
            {
                nNx = pPixel->nX + dx[nIndex];
                nNy = pPixel->nY + dy[nIndex];
                if (!CheckXY(pWatershedImage, nNx, nNy))
                    continue;

                pNewPixel = GetPixelXY(pWatershedImage, nNx, nNy);
                //����Ǳ��������Ѿ����ֵ�����������
                if (pNewPixel->lRegionNo == newRegionNo)
                    continue;

                //������Ѿ�����û������,��¼����������������
                if (pNewPixel->bDrown)
                {
                    pNeighbors->insert(pNewPixel->lRegionNo);
                    continue;
                }

                if (pNewPixel->btAsh != btAsh)
                    continue;

                //�����·��ֵ�����,�����������,��������û
                pNewPixel->bDrown = true;
                pNewPixel->lRegionNo = newRegionNo;
                pNewList->push_back(pNewPixel);
                pPixelsFound->push_back(pNewPixel);
            }
        }
        pSwapList = pCurrentList;
        pCurrentList = pNewList;
        pNewList = pSwapList;
    }
    return true;
}

/****************************************************************************
* �������㷨����ָ�����������
* ������pRegion         Ҫ����������
*       pPixelsFound    ������������
****************************************************************************/
bool GetRegionPixels(
    WatershedImage *pWatershedImage, 
    WatershedRegion *pRegion, 
    list<WatershedPixel *> *pPixelsFound
)
{
    //bool bRetn = false;
    static int dx[8] = {-1, 0, 1, 1, 1, 0, -1, -1};
    static int dy[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
    static list<WatershedPixel *> list1, list2;
    static set<WatershedPixel *> pixelsDone;

    list<WatershedPixel *>::iterator itPix;
    list<WatershedPixel *> *pCurrentList, *pNewList, *pSwapList;
    set<WatershedPixel *>::_Pairib pairIb;
    WatershedPixel *pNewPixel, *pPixel;
    WatershedRegion *pNewRegion = NULL;
    RegionNo regionNo;
    int nIndex, nNx, nNy;

    pCurrentList = &list1;
    pNewList = &list2;
    regionNo = pRegion->regionNo;

    pCurrentList->clear();
    pixelsDone.clear();
    pCurrentList->push_back(pRegion->pPixel);
    pPixelsFound->push_back(pRegion->pPixel);
    pixelsDone.insert(pRegion->pPixel);

    while (pCurrentList->size() > 0)
    {
        pNewList->clear();
        FOR_EACH(pCurrentList, itPix)
        {
            pPixel = *itPix;
            for (nIndex = 0; nIndex < 8; nIndex++)
            {
                nNx = pPixel->nX + dx[nIndex];
                nNy = pPixel->nY + dy[nIndex];
                if (!CheckXY(pWatershedImage, nNx, nNy))
                    continue;

                pNewPixel = GetPixelXY(pWatershedImage, nNx, nNy);
                if (pNewPixel->lRegionNo != regionNo)
                    continue;

                pairIb = pixelsDone.insert(pNewPixel);
                if (!pairIb.second)
                    continue;

                pNewList->push_back(pNewPixel);
                pPixelsFound->push_back(pNewPixel);
            }
        }
        pSwapList = pCurrentList;
        pCurrentList = pNewList;
        pNewList = pSwapList;
    }

    //bRetn = true;
    return true;
}

/****************************************************************************
* �ϲ�������ͬ����
* ������pDestRegion   �ϲ�����Ŀ������
*       pSrcRegion    ���ϲ�������
****************************************************************************/
bool CombineRegion(
    WatershedImage *pWatershedImage, 
    WatershedRegion *pDestRegion, 
    WatershedRegion *pSrcRegion
)
{
    static list<WatershedPixel *> pixelList;

    bool bRetn = false;
    bool b;
    list<WatershedPixel *>::iterator itPix;

    WatershedPixel *pPixel;
    RegionNo destRegionNo;
    BYTE btLowest = 255;

    //���ϲ���������������ͬ
    if (pDestRegion->regionNo == pSrcRegion->regionNo)
        goto Exit0;

    pixelList.clear();
    destRegionNo = pDestRegion->regionNo;

    //�趨���ϲ�����������
    if (pSrcRegion->btLowest < pDestRegion->btLowest)
    {
        btLowest = pSrcRegion->btLowest;
    }
    else
    {
        btLowest = pDestRegion->btLowest;
    }

    // �������ϲ��������������
    b = GetRegionPixels(pWatershedImage, pSrcRegion, &pixelList);
    if (!b)
        goto Exit0;

    // ���ı��ϲ��������ص�������
    FOR_EACH(&pixelList, itPix)
    {
        pPixel = (*itPix);
        pPixel->lRegionNo = destRegionNo;
    }

    pDestRegion->btLowest = btLowest;
    pDestRegion->lPixelCount += pSrcRegion->lPixelCount;
    pDestRegion->lTotalAsh += pSrcRegion->lTotalAsh;

    // ��ͼ����ɾ�����ϲ�������
    pWatershedImage->regionMap.erase(pSrcRegion->regionNo);
    delete pSrcRegion;

    bRetn = true;
Exit0:
    return bRetn;
}

bool CreateNewRegion(
    WatershedImage *pWatershedImage,
    list<WatershedPixel *> *pNewPixels,
    RegionNo newRegionNo,
    BYTE btLowest
)
{
    bool bRetn = false;
    WatershedRegion *pNewRegion;
    WatershedPixel *pPixel;
    list<WatershedPixel *>::iterator itPix;
    long lTotalAsh = 0;

    FOR_EACH(pNewPixels, itPix)
    {
        pPixel = *itPix;
        lTotalAsh += pPixel->btAsh;
    }

    pNewRegion = new WatershedRegion();
    if (NULL == pNewRegion)
        goto Exit0;
    pNewRegion->btLowest = btLowest;
    pNewRegion->lPixelCount = (long)pNewPixels->size();
    pNewRegion->regionNo = newRegionNo;
    pNewRegion->pPixel = *(pNewPixels->begin());
    pNewRegion->lTotalAsh = lTotalAsh;
    pWatershedImage->regionMap.insert(
        make_pair(newRegionNo, pNewRegion));

    bRetn = true;
Exit0:
    return bRetn;
}

/****************************************************************************
* ���ڽ�����û�����غϲ����������򣬻򴴽��µ����� 
* δ���ϲ�������֮������Ϊ�����˵̰�
* ������btAsh         ��ǰ��û�����
*       pNewPixels    ����û������
*       pNeighbors    ����û�������ڵ�����
*       newRegionNo   ����û������������ʱ������
****************************************************************************/
bool CombineAndDam(
    WatershedImage *pWatershedImage,
    BYTE btAsh,
    list<WatershedPixel *> *pNewPixels,
    set<RegionNo> *pNeighbors,
    RegionNo newRegionNo
)
{
    bool bRetn = false;
    set<WatershedRegion *>::iterator itRegion;
    set<RegionNo>::iterator itRegionNo;
    map<RegionNo, WatershedRegion *>::iterator itPair;
    WatershedRegion *pRegion;
    WatershedRegion *pNewRegion = NULL;
    WatershedRegion *pDeepRegionToCombine = NULL;
    RegionNo regionNo;
    long lDeepRegionCount = 0;
    bool b, bCombined = false;
    BYTE btAverage = 0;

    if (pNewPixels->size() == 0)
        goto Exit0;

    // ���μ������û������ٽ�����
    FOR_EACH(pNeighbors, itRegionNo)
    {
        regionNo = (*itRegionNo);
        itPair = pWatershedImage->regionMap.find(regionNo);
        if (itPair == pWatershedImage->regionMap.end())
            goto Exit0;
        pRegion = (*itPair).second;
        if (btAsh < pRegion->btLowest)
            goto Exit0;

        // ����ٽ�������Ϊǳˮ�ػ�Сˮ�أ�������ϲ�
        if ((btAsh - pRegion->btLowest < pWatershedImage->bTolerance) ||
            (pRegion->lPixelCount < MIN_REGIONPIXEL_COUNT))
        {
            if (!bCombined)
            {
                b = AddPixelsToRegion(pNewPixels, pRegion);
                if (!b)
                    goto Exit0;
                bCombined = true;
                pNewRegion = pRegion;
            }
            else
            {
                b = CombineRegion(pWatershedImage, pNewRegion, pRegion);
                if (!b)
                    goto Exit0;
            }
            continue;
        }

        //����ٽ�������Ϊ��ˮ��,��ѡ��ƽ���Ҷ��뵱ǰ��ӽ���һ�����кϲ�
        if (NULL == pDeepRegionToCombine)
        {
            pDeepRegionToCombine = pRegion;
            btAverage = (BYTE)(pRegion->lTotalAsh / pRegion->lPixelCount);
            lDeepRegionCount += 1;
        }
        else
        {
            if ((BYTE)(pRegion->lTotalAsh / pRegion->lPixelCount) > btAverage)
            {
                pDeepRegionToCombine = pRegion;
            }
            lDeepRegionCount += 1;
        }
    }

    // �����������С����ˮ��,
    // �������û������ͬ���ڵ�ǳˮ��һͬ����ϲ�
    if (NULL != pDeepRegionToCombine)
    {
        if (!bCombined)
        {
            b = AddPixelsToRegion(pNewPixels, pDeepRegionToCombine);
            if (!b)
                goto Exit0;
            bCombined = true;
        }
        else
        {
            b = CombineRegion(pWatershedImage, pDeepRegionToCombine, pNewRegion);
            if (!b)
                goto Exit0;
        }
    }
    // ����, �������û����û�����κ���������
    // Ϊ����û���򴴽��µ�����
    else
    {
        if (!bCombined)
        {
            b = CreateNewRegion(pWatershedImage, pNewPixels, newRegionNo, btAsh);
            if (!b)
                goto Exit0;
        }
    }

    bRetn = true;
Exit0:
    return bRetn;
}

/****************************************************************************
* ��û��ָ�����
* ������btAsh        ��û�����
****************************************************************************/
bool Flood(WatershedImage *pWatershedImage, BYTE btAsh)
{
    static set<RegionNo> neighbor;
    static list<WatershedPixel *> pixelsFound;

    bool bRetn = false;
    bool b;
    long lIndex, lCount;
    WatershedPixel *pPixel;
    RegionNo tmpRegionNo;

    lCount = pWatershedImage->nHeight * pWatershedImage->nWidth;
    for (lIndex = 0; lIndex < lCount; lIndex++)
    {
        pPixel = &pWatershedImage->pixels[lIndex];
        if (pPixel->btAsh != btAsh)
            continue;
        if (pPixel->bDrown)
            continue;
        neighbor.clear();
        pixelsFound.clear();
        // Ѱ������û�ĵ�������
        b = SeedSearchPixel(
            pWatershedImage, pPixel, 
            &neighbor, &pixelsFound, &tmpRegionNo);

        if (!b)
            goto Exit0;

        // ����������û���������ٽ�����ϲ�������
        b = CombineAndDam(
            pWatershedImage, btAsh, 
            &pixelsFound, &neighbor, tmpRegionNo);
        if (!b)
            goto Exit0;
    }
    bRetn = true;
Exit0:
    return bRetn;
}

/****************************************************************************
* ���ָ�õ��������ò�ͬ����ɫ���������ͼ����
* ������imageOut        �����ͼ��
****************************************************************************/
bool OutputToByteImage(WatershedImage *pWatershedImage, ByteImage imageOut)
{
    static set<RegionNo> neighbor;
    bool bRetn = false;
    map<RegionNo, WatershedRegion *>::iterator itPair;
    long lIndex, lCount;
    WatershedPixel *pPixel;
    WatershedRegion *pRegion = NULL;
    BYTE btAsh;

    lCount = pWatershedImage->nHeight * pWatershedImage->nWidth;
    for (lIndex = 0; lIndex < lCount; lIndex++)
    {
        pPixel = &pWatershedImage->pixels[lIndex];
        //���������Ų����������ɫ
        itPair = pWatershedImage->regionMap.find(pPixel->lRegionNo);
        if (itPair == pWatershedImage->regionMap.end())
            goto Exit0;
        pRegion = (*itPair).second;
        btAsh = (BYTE)(pRegion->lTotalAsh / pRegion->lPixelCount);

        imageOut[lIndex * 4  + 0] = btAsh;
        imageOut[lIndex * 4  + 1] = btAsh;
        imageOut[lIndex * 4  + 2] = btAsh;
        imageOut[lIndex * 4  + 3] = 255;
    }

    bRetn = true;
Exit0:
    return bRetn;
}


/********************************************************************************
* ��ˮ��ָ��㷨
* ������imageIn     �����ͼ��
*       imageOut    �����ͼ��
*       w, h        ͼ��ߴ�
********************************************************************************/
void __stdcall Watershed(
    BYTE *imageIn, BYTE * imageOut, 
    unsigned int w, unsigned int h
                         )
{
    WatershedImage watershedImage;
    bool bRetnSuccess;
    int nAsh;
    BYTE btAsh;

    bRetnSuccess = InitWatershedImage(&watershedImage, imageIn, w, h, TOLERANCE);
    if (!bRetnSuccess)
        goto Exit0;

    for (nAsh = 0; nAsh < 255; nAsh += STEP_ASH)
    {
        btAsh = (BYTE)(nAsh & 0xFF);
        bRetnSuccess = Flood(&watershedImage, btAsh);
        if (!bRetnSuccess)
            goto Exit0;
    }
    bRetnSuccess = OutputToByteImage(&watershedImage, imageOut);
    if (!bRetnSuccess)
        goto Exit0;

    bRetnSuccess = FinalizeWatershedImage(&watershedImage);
    if (!bRetnSuccess)
        goto Exit0;

Exit0:
    return;
}

}
#endif
