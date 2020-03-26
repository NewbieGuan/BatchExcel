#include "stdafx.h"
#include "Pictrue.h"


Pictrue::Pictrue()
{
}


Pictrue::~Pictrue()
{
}


//���ò������������д���������
void Pictrue::SetParameter(double m_resolution, CString m_savePath, vector<CString> PicPaths)
{
	resolution = m_resolution;
	savePath = m_savePath;
	vecPicPaths = PicPaths;
}


//������ʼ��
void Pictrue::Initialize()
{
	valueCount = 0;            // ����
	meanArea = 0;            // ƽ��������
	meanLength = 0;          // ƽ���ܳ����
	ratioArea = 0;           // ����������
	meanMinDia = 0;          // ƽ����С����
	meanMaxDia = 0;          // ƽ���������
	maxBigDia = 0;           // ��������
	minBigDia = 0;           // ������С��
	maxSmallDia = 0;         // �������С��
	minSmallDia = 0;         // ������СС��
	maxLength = 0;           // ��������ܳ�
	minLength = 0;           // ������С�ܳ�
	maxArea = 0;             // ����������
	minArea = 0;             // ������С���
	sumBigDia = 0;           // ���ֱ��֮��
	sumSmallDia = 0;         // ��Сֱ��֮��
	sumLength = 0;           // �ܳ���
	sumArea = 0;             // �����
	bigDiaSD = 0;            // ���ֱ����׼��
	smallDiaSD = 0;          // ��Сֱ����׼��
	lengthSD = 0;            // �ܳ��ı�׼��
	areaSD = 0;              // ����ı�׼��
	areaStrip = 0;           // ÿһ�������
	vecArea.clear();         // ���
	vecLength.clear();       // �ܳ�
	vecMaxDia.clear();       // ÿ���������
	vecMinDia.clear();       // ÿ����С����
	PicSavePath = "";           //������ͼ·��
	ExcelSavePath = "";         //����excel���·��
	abandonArea = 0;         //�����������������ظ���
}


// ��ȷ���㺯��
void Pictrue::ComputeAccuracy(Mat srcImg)
{
	Mat dstImg, binImg;
	Thresh(srcImg, dstImg);
	cvtColor(dstImg, dstImg, COLOR_BGR2GRAY);
	threshold(dstImg, binImg, 120, 255, CV_THRESH_BINARY);

	// ��������
	vector < vector < cv::Point >> contours;
	vector<Vec4i> hierarchy;
	findContours(binImg, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	//Mat dstImg2 = Mat::zeros(srcImg.rows, srcImg.cols, CV_8UC3);

	double area = 0;   //ÿ���������������ʱ������
	double length = 0;   //ÿ���������ܳ�����ʱ������
	double areaSum = 0.0;   //���������ʱ������
	double lengthSum = 0.0;   //���ܳ�����ʱ������
	double maxRadius = 0.0;   // �������
	double minRadius = 0.0;   // ��С����
	double radiusMaxSum = 0.0;   // �������֮��
	double radiusMinSum = 0.0;   // ��С����֮��
	// ��С��Ӿ���
	double l1 = 0.0;
	double l2 = 0.0;
	double bigL = 0.0;   //��
	double smallL = 0.0;   //��

	// �������ж�������
	for (auto index = 0; index < contours.size(); index++)
	{
		area = contourArea(contours[index]);
		length = arcLength(contours[index], true);

		// ��С��Ӿ��μ���
		RotatedRect rect = minAreaRect(contours[index]);
		Point2f P[4];
		rect.points(P);
		Size s = rect.size;
		l1 = s.width;
		l2 = s.height;
		if (l1 > l2)
		{
			bigL = l1 * resolution;
			smallL = l2 * resolution;
		}
		else
		{
			bigL = l2 * resolution;
			smallL = l1 * resolution;
		}
		minRadius = smallL;  // �õ���С����

		double maxLength = 0.0;
		double curLenth = 0.0;
		// ÿ��������ѭ����ȷ�����������
		for (auto i = 0; i < contours[index].size(); i++)
		{
			for (auto j = 0; j < contours[index].size(); j++)
			{
				curLenth = sqrt((contours[index][i].x - contours[index][j].x)*(contours[index][i].x - contours[index][j].x) +
					(contours[index][i].y - contours[index][j].y)*(contours[index][i].y - contours[index][j].y));
				if (curLenth > maxLength)
					maxLength = curLenth;
			}
		}
		maxRadius = maxLength * resolution;// �õ��������

		valueCount++;
		areaSum += area;
		lengthSum += length;
		radiusMaxSum += maxRadius;
		radiusMinSum += minRadius;

		area = area * resolution*resolution;
		length = length * resolution;

		vecArea.push_back(area);           // ���
		vecLength.push_back(length);       // �ܳ�
		vecMaxDia.push_back(maxRadius);    // ÿ���������
		vecMinDia.push_back(minRadius);    // ÿ����С����
		//cv::drawContours(dstImg2, contours, index, color_red, CV_FILLED, 8, hierarchy);
	}
	//USES_CONVERSION;
	//cv::String PicSavePath_cv = W2A(PicSavePath);
	//cv::imwrite(PicSavePath_cv, dstImg2);

	// ******************��ƽ���� ������ʡ� ���������������
	meanMaxDia = radiusMaxSum / valueCount;                                 // ƽ���������
	meanMinDia = radiusMinSum / valueCount;                                 // ƽ����С����
	meanArea = areaSum / valueCount * resolution * resolution;          // ƽ�����
	meanLength = lengthSum / valueCount * resolution;                     // ƽ���ܳ�
	ratioArea = areaSum / (srcImg.rows*srcImg.cols - abandonArea) * 100;                  // �����
	areaStrip = (srcImg.rows*srcImg.cols - abandonArea) *resolution*resolution;        // һС�������

	// ******************���ܼơ�
	sumBigDia = radiusMaxSum;
	sumSmallDia = radiusMinSum;
	sumLength = lengthSum * resolution;
	sumArea = areaSum * resolution*resolution;

	// *****************����� ����С�� ����׼�
	for (auto i = 0; i < vecArea.size(); i++)
	{
		// ѭ����һ�γ�ʼ��
		if (i == 0)
		{
			maxBigDia = vecMaxDia[i];
			minBigDia = vecMaxDia[i];

			maxSmallDia = vecMinDia[i];
			minSmallDia = vecMinDia[i];

			maxLength = vecLength[i];
			minLength = vecLength[i];

			maxArea = vecArea[i];
			minArea = vecArea[i];
		}
		else
		{
			if (maxBigDia < vecMaxDia[i])
				maxBigDia = vecMaxDia[i];
			if (minBigDia > vecMaxDia[i])
				minBigDia = vecMaxDia[i];

			if (maxSmallDia < vecMinDia[i])
				maxSmallDia = vecMinDia[i];
			if (minSmallDia > vecMinDia[i])
				minSmallDia = vecMinDia[i];

			if (maxLength < vecLength[i])
				maxLength = vecLength[i];
			if (minLength > vecLength[i])
				minLength = vecLength[i];

			if (maxArea < vecArea[i])
				maxArea = vecArea[i];
			if (minArea > vecArea[i])
				minArea = vecArea[i];

		}
		// ��׼��ļ���
		bigDiaSD += (vecMaxDia[i] - meanMaxDia)*(vecMaxDia[i] - meanMaxDia) / valueCount;
		smallDiaSD += (vecMinDia[i] - meanMinDia)*(vecMinDia[i] - meanMinDia) / valueCount;
		lengthSD += (vecLength[i] - meanLength)*(vecLength[i] - meanLength) / valueCount;
		areaSD += (vecArea[i] - meanArea)*(vecArea[i] - meanArea) / valueCount;
	}

	bigDiaSD = sqrt(bigDiaSD);
	smallDiaSD = sqrt(smallDiaSD);
	lengthSD = sqrt(lengthSD);
	areaSD = sqrt(areaSD);
}


void Pictrue::Thresh(Mat &srcImag, Mat &dstImag)
{
	dstImag = srcImag.clone();
	int height = dstImag.rows;
	int width = dstImag.cols;
	int row, col;
	if (dstImag.channels() == 3)
	{
		for (row = 0; row < height; ++row)
		{
			for (col = 0; col < width; ++col)
			{
				int b = dstImag.at<Vec3b>(row, col)[0];
				int g = dstImag.at<Vec3b>(row, col)[1];
				int r = dstImag.at<Vec3b>(row, col)[2];
				if (b < 10 && g < 10 && r > 245)  //������ɫ������
				{
					dstImag.at<Vec3b>(row, col)[0] = 255;
					dstImag.at<Vec3b>(row, col)[1] = 255;
					dstImag.at<Vec3b>(row, col)[2] = 255;
				}
				else if (b >245 && g >245 && r > 245)  //������ɫ������
				{
					++abandonArea;
					dstImag.at<Vec3b>(row, col)[0] = 0;
					dstImag.at<Vec3b>(row, col)[1] = 0;
					dstImag.at<Vec3b>(row, col)[2] = 0;
				}
				else if (b < 10 && g > 245 && r < 10)  //������ɫ������
				{
					++abandonArea;
					dstImag.at<Vec3b>(row, col)[0] = 0;
					dstImag.at<Vec3b>(row, col)[1] = 0;
					dstImag.at<Vec3b>(row, col)[2] = 0;
				}
				else  //����ȫ������ɫ
				{
					dstImag.at<Vec3b>(row, col)[0] = 0;
					dstImag.at<Vec3b>(row, col)[1] = 0;
					dstImag.at<Vec3b>(row, col)[2] = 0;
				}
			}
		}
	}
}


// ��������浽excel�ļ�
void Pictrue::WriteResultToExcelAccuracy()

{
	// ��ʼ�����
	CoUninitialize();
	if (CoInitialize(NULL) == S_FALSE)
	{
		AfxMessageBox(_T("��ʼ��COM֧�ֿ�ʧ�ܣ�"));
	}

	COleVariant covOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
	if (!app.CreateDispatch(_T("Excel.Application")))
	{
		AfxMessageBox(_T("�޷�����ExcelӦ�ã�"));
	}

	books = app.get_Workbooks();
	book = books.Add(covOptional);
	sheets = book.get_Worksheets();
	sheet = sheets.get_Item(COleVariant((short)1));

	//���ж��뷽ʽ
	val.vt = VT_I2;
	val.iVal = -4108;


	// *****************************����ͷ��********************************
	//�������Ϊ��A��1���ͣ�D��1����������Ԫ�� 
	range = sheet.get_Range(COleVariant(_T("A1")), COleVariant(_T("E1")));
	range.put_RowHeight(_variant_t((long)50));  //�����п�
	range.Merge(_variant_t((long)0));//�ϲ���Ԫ��

	//���ж���
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);

	//���õ�Ԫ������
	range.put_Value2(COleVariant(_T("���ͳ�Ʊ���")));

	//�����������
	font = range.get_Font();
	font.put_Bold(COleVariant((short)TRUE));
	font.put_Name(_variant_t("����"));
	font.put_Size(_variant_t(30));


	// *************************������ʱ�䡿********************************
	range = sheet.get_Range(COleVariant(_T("A2")), COleVariant(_T("A2")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("����ʱ�䣺")));

	CTime curTime = CTime::GetCurrentTime();
	CString curTimeCStr;

	curTimeCStr = curTime.Format(_T("%Y.%m.%d"));
	range = sheet.get_Range(COleVariant(_T("B2")), COleVariant(_T("B2")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(curTimeCStr));

	curTimeCStr = curTime.Format(_T("%H:%M"));
	range = sheet.get_Range(COleVariant(_T("C2")), COleVariant(_T("C2")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(curTimeCStr));


	// *************************��ͳ�ƽ����********************************
	range = sheet.get_Range(COleVariant(_T("A3")), COleVariant(_T("A3")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("��ͳ�ơ�")));

	range = sheet.get_Range(COleVariant(_T("B4")), COleVariant(_T("B4")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("���ֱ��/um")));

	range = sheet.get_Range(COleVariant(_T("C4")), COleVariant(_T("C4")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("��Сֱ��/um")));

	range = sheet.get_Range(COleVariant(_T("D4")), COleVariant(_T("D4")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("�ܳ�/um")));

	range = sheet.get_Range(COleVariant(_T("E4")), COleVariant(_T("E4")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("���/um2")));

	range = sheet.get_Range(COleVariant(_T("A5")), COleVariant(_T("A5")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("ƽ��")));

	range = sheet.get_Range(COleVariant(_T("A6")), COleVariant(_T("A6")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("��׼��")));

	range = sheet.get_Range(COleVariant(_T("A7")), COleVariant(_T("A7")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("���")));

	range = sheet.get_Range(COleVariant(_T("A8")), COleVariant(_T("A8")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("��С")));

	range = sheet.get_Range(COleVariant(_T("A9")), COleVariant(_T("A9")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("�ܼ�")));



	// ******************��ƽ����
	// ���ֱ��
	CString strResult;
	strResult.Format(_T("%.0f"), meanMaxDia);
	range = sheet.get_Range(COleVariant(_T("B5")), COleVariant(_T("B5")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// ��Сֱ��
	strResult.Format(_T("%.0f"), meanMinDia);
	range = sheet.get_Range(COleVariant(_T("C5")), COleVariant(_T("C5")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// �ܳ�
	strResult.Format(_T("%.0f"), meanLength);
	range = sheet.get_Range(COleVariant(_T("D5")), COleVariant(_T("D5")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// ���
	strResult.Format(_T("%.0f"), meanArea);
	range = sheet.get_Range(COleVariant(_T("E5")), COleVariant(_T("E5")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// ******************����׼�
	// ���ֱ��
	strResult.Format(_T("%.0f"), bigDiaSD);
	range = sheet.get_Range(COleVariant(_T("B6")), COleVariant(_T("B6")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// ��Сֱ��
	strResult.Format(_T("%.0f"), smallDiaSD);
	range = sheet.get_Range(COleVariant(_T("C6")), COleVariant(_T("C6")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// �ܳ�
	strResult.Format(_T("%.0f"), lengthSD);
	range = sheet.get_Range(COleVariant(_T("D6")), COleVariant(_T("D6")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// ���
	strResult.Format(_T("%.0f"), areaSD);
	range = sheet.get_Range(COleVariant(_T("E6")), COleVariant(_T("E6")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));


	// ******************�����
	// ���ֱ��
	strResult.Format(_T("%.0f"), maxBigDia);
	range = sheet.get_Range(COleVariant(_T("B7")), COleVariant(_T("B7")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// ��Сֱ��
	strResult.Format(_T("%.0f"), maxSmallDia);
	range = sheet.get_Range(COleVariant(_T("C7")), COleVariant(_T("C7")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// �ܳ�
	strResult.Format(_T("%.0f"), maxLength);
	range = sheet.get_Range(COleVariant(_T("D7")), COleVariant(_T("D7")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// ���
	strResult.Format(_T("%.0f"), maxArea);
	range = sheet.get_Range(COleVariant(_T("E7")), COleVariant(_T("E7")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// ******************����С��
	// ���ֱ��
	strResult.Format(_T("%.0f"), minBigDia);
	range = sheet.get_Range(COleVariant(_T("B8")), COleVariant(_T("B8")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// ��Сֱ��
	strResult.Format(_T("%.0f"), minSmallDia);
	range = sheet.get_Range(COleVariant(_T("C8")), COleVariant(_T("C8")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// �ܳ�
	strResult.Format(_T("%.0f"), minLength);
	range = sheet.get_Range(COleVariant(_T("D8")), COleVariant(_T("D8")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// ���
	strResult.Format(_T("%.0f"), minArea);
	range = sheet.get_Range(COleVariant(_T("E8")), COleVariant(_T("E8")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// ******************���ܼơ�
	// ���ֱ��
	strResult.Format(_T("%.0f"), sumBigDia);
	range = sheet.get_Range(COleVariant(_T("B9")), COleVariant(_T("B9")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// ��Сֱ��
	strResult.Format(_T("%.0f"), sumSmallDia);
	range = sheet.get_Range(COleVariant(_T("C9")), COleVariant(_T("C9")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// �ܳ�
	strResult.Format(_T("%.0f"), sumLength);
	range = sheet.get_Range(COleVariant(_T("D9")), COleVariant(_T("D9")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));

	// ���
	strResult.Format(_T("%.0f"), sumArea);
	range = sheet.get_Range(COleVariant(_T("E9")), COleVariant(_T("E9")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strResult));


	// *************************������Ϣ��********************************
	range = sheet.get_Range(COleVariant(_T("A13")), COleVariant(_T("A13")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("������Ϣ��")));

	range = sheet.get_Range(COleVariant(_T("A14")), COleVariant(_T("A14")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("No.")));

	range = sheet.get_Range(COleVariant(_T("B14")), COleVariant(_T("B14")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("���ֱ��/um")));

	range = sheet.get_Range(COleVariant(_T("C14")), COleVariant(_T("C14")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("��Сֱ��/um")));

	range = sheet.get_Range(COleVariant(_T("D14")), COleVariant(_T("D14")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("�ܳ�/um")));

	range = sheet.get_Range(COleVariant(_T("E14")), COleVariant(_T("E14")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("���/um2")));


	long irow;
	CRange start_range, write_range;
	CString strValue;
	long sizeVec = vecArea.size();
	for (irow = 1; irow <= sizeVec; irow++)
	{
		// No.
		strValue.Format(_T("%ld"), irow);
		COleVariant new_value0(strValue);
		start_range = sheet.get_Range(COleVariant(_T("A14")), covOptional);
		write_range = start_range.get_Offset(COleVariant((long)irow), COleVariant((long)0));
		write_range.put_Value2(new_value0);
		write_range.put_HorizontalAlignment(val);

		// ���ֱ��/um
		strValue.Format(_T("%.0f"), vecMaxDia[irow - 1]);
		COleVariant new_value1(strValue);
		write_range = start_range.get_Offset(COleVariant((long)irow), COleVariant((long)1));
		write_range.put_Value2(new_value1);
		write_range.put_HorizontalAlignment(val);

		// ��Сֱ��/um
		strValue.Format(_T("%.0f"), vecMinDia[irow - 1]);
		COleVariant new_value2(strValue);
		write_range = start_range.get_Offset(COleVariant((long)irow), COleVariant((long)2));
		write_range.put_Value2(new_value2);
		write_range.put_HorizontalAlignment(val);

		// �ܳ�/um
		strValue.Format(_T("%.0f"), vecLength[irow - 1]);
		COleVariant new_value3(strValue);
		write_range = start_range.get_Offset(COleVariant((long)irow), COleVariant((long)3));
		write_range.put_Value2(new_value3);
		write_range.put_HorizontalAlignment(val);

		// ���/um2
		strValue.Format(_T("%.0f"), vecArea[irow - 1]);
		COleVariant new_value4(strValue);
		write_range = start_range.get_Offset(COleVariant((long)irow), COleVariant((long)4));
		write_range.put_Value2(new_value4);
		write_range.put_HorizontalAlignment(val);
	}


	// ֵ
	write_range = start_range.get_Offset(COleVariant((long)irow + 1), COleVariant((long)1));
	write_range.put_ColumnWidth(_variant_t((long)12));
	write_range.put_HorizontalAlignment(val);
	write_range.put_Value2(COleVariant(_T("ֵ")));

	// ��λ
	write_range = start_range.get_Offset(COleVariant((long)irow + 1), COleVariant((long)2));
	write_range.put_ColumnWidth(_variant_t((long)12));
	write_range.put_HorizontalAlignment(val);
	write_range.put_Value2(COleVariant(_T("��λ")));


	// �����
	write_range = start_range.get_Offset(COleVariant((long)irow + 2), COleVariant((long)0));
	write_range.put_ColumnWidth(_variant_t((long)12));
	write_range.put_HorizontalAlignment(val);
	write_range.put_Value2(COleVariant(_T("�����")));

	strValue.Format(_T("%.0f"), meanArea*valueCount);
	COleVariant new_value_areaSum(strValue);
	write_range = start_range.get_Offset(COleVariant((long)irow + 2), COleVariant((long)1));
	write_range.put_ColumnWidth(_variant_t((long)12));
	write_range.put_HorizontalAlignment(val);
	write_range.put_Value2(COleVariant(new_value_areaSum));

	write_range = start_range.get_Offset(COleVariant((long)irow + 2), COleVariant((long)2));
	write_range.put_ColumnWidth(_variant_t((long)12));
	write_range.put_HorizontalAlignment(val);
	write_range.put_Value2(COleVariant(_T("um2")));


	// �����������
	write_range = start_range.get_Offset(COleVariant((long)irow + 3), COleVariant((long)0));
	write_range.put_ColumnWidth(_variant_t((long)12));
	write_range.put_HorizontalAlignment(val);
	write_range.put_Value2(COleVariant(_T("�����������")));

	strValue.Format(_T("%.0f"), areaStrip);
	COleVariant new_value_areaStrip(strValue);
	write_range = start_range.get_Offset(COleVariant((long)irow + 3), COleVariant((long)1));
	write_range.put_ColumnWidth(_variant_t((long)12));
	write_range.put_HorizontalAlignment(val);
	write_range.put_Value2(COleVariant(new_value_areaStrip));

	write_range = start_range.get_Offset(COleVariant((long)irow + 3), COleVariant((long)2));
	write_range.put_ColumnWidth(_variant_t((long)12));
	write_range.put_HorizontalAlignment(val);
	write_range.put_Value2(COleVariant(_T("um2")));


	// ����
	range = sheet.get_Range(COleVariant(_T("A10")), COleVariant(_T("A10")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("����/��")));

	strValue.Format(_T("%ld"), valueCount);
	range = sheet.get_Range(COleVariant(_T("B10")), COleVariant(_T("B10")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strValue));



	//�����
	range = sheet.get_Range(COleVariant(_T("A11")), COleVariant(_T("A11")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(_T("�����/%")));

	strValue.Format(_T("%.2f"), ratioArea);
	range = sheet.get_Range(COleVariant(_T("B11")), COleVariant(_T("B11")));
	range.put_ColumnWidth(_variant_t((long)12));
	range.put_HorizontalAlignment(val);
	range.put_VerticalAlignment(val);
	range.put_Value2(COleVariant(strValue));


	//�����ʾ
	//app.put_Visible(TRUE);
	book.put_Saved(false);

	//����ʾ��ʾ�Ի���
	app.put_DisplayAlerts(false);


	//����excel
	//book.SaveAs(COleVariant(ExcelSavePath),
	//	_variant_t(39),        //07��
	//	_variant_t(vtMissing),
	//	_variant_t(vtMissing),
	//	_variant_t(vtMissing),
	//	_variant_t(vtMissing),
	//	0,
	//	_variant_t(vtMissing),
	//	_variant_t(vtMissing),
	//	_variant_t(vtMissing),
	//	_variant_t(vtMissing),
	//	_variant_t(vtMissing));
	book.SaveCopyAs(COleVariant(ExcelSavePath));
	book.put_Saved(TRUE);

	//��β���ͷ�

	//start_range.ReleaseDispatch();
	//write_range.ReleaseDispatch();

	//book.ReleaseDispatch();
	//books.ReleaseDispatch();
	//sheet.ReleaseDispatch();
	//sheets.ReleaseDispatch();
	//range.ReleaseDispatch();
	//font.ReleaseDispatch();
	//book.Close(covOptional, covOptional, covOptional);	   // �ر�Workbook����
	//books.Close();										   // �ر�Workbooks����

	//app.Quit();
	//app.ReleaseDispatch();

	range.ReleaseDispatch();
	sheet.ReleaseDispatch();
	sheets.ReleaseDispatch();

	start_range.ReleaseDispatch();
	write_range.ReleaseDispatch();

	book.ReleaseDispatch();								   // �ͷ�Workbook����
	books.ReleaseDispatch();							   // �ͷ�Workbooks����
	book.Close(covOptional, covOptional, covOptional);	   // �ر�Workbook����
	books.Close();										   // �ر�Workbooks����

	app.Quit();
	app.ReleaseDispatch();
}


//�õ��������·��
void Pictrue::SetFilePath(CString picpath, CString savepath)
{
	CString FileName, FileSuffix;
	FileName = picpath.Right(picpath.GetLength() - picpath.ReverseFind('\\') - 1);//��·���н�ȡ�ļ��� "picture_1.bmp"
	FileName = FileName.Left(FileName.ReverseFind('.')); //ȥ����׺ "picture_1"
	FileSuffix = picpath.Right(picpath.GetLength() - picpath.ReverseFind('.')); //��ȡ�ļ��ĺ�׺ ".bmp"

	PicSavePath = savepath + "\\" + FileName + "-color" + FileSuffix;  //"...\\picture_1-color.bmp"
	ExcelSavePath = savepath + "\\" + FileName;
	ExcelSavePath += ".xls";   //"...\\picture_1.xls"
}


//�ܴ���
//void Pictrue::Progressing()
//{
//	int path_size = vecPicPaths.size();
//	for (int num = 0; num < path_size; ++num)
//	{
//		CString picpath = vecPicPaths[num];
//		USES_CONVERSION;
//		cv::String picpath_cv = W2A(picpath);
//		Mat srcImg = imread(picpath_cv, 0);
//
//		SetFilePath(picpath, savePath);   //�õ��ļ������ļ���׺
//		Initialize();   //ÿ���ȳ�ʼ������
//		ComputeAccuracy(srcImg);   // ������ͼ����ȷ������������
//		WriteResultToExcelAccuracy();   // ��������浽excel�ļ�
//	}
//	AfxMessageBox(_T("������ɣ�"));
//}