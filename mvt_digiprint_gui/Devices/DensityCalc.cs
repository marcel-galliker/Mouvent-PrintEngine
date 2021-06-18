using RX_DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using static RX_DigiPrint.Devices.ColorConversion;

namespace RX_DigiPrint.Devices
{
    public class DensityCalc
    {
        public const double DeSpreadFactor = 50;
        public const double DensityCorrectionFactor = 1.42;
        public readonly double[] WeightList = {0.8, 0.7, 0.6, 0.5};


        #region First Run

        /// <summary>
        /// Calculates the Average each of the three CIE L*a*b* values
        /// </summary>
        /// <param name="MeasureList">Measure Results of current Measurement, MeasureList[DensityStrip][MeasurePoint]SpectroResultStruct</param>
        /// <param name="Use120b">Use (120 - b) instead of L* (for Yellow), default = false</param>
        /// <returns>CieLabStruct[]: averages of all L*a*b* for each DensityStrip</returns>
        public CieLabStruct[] CalcAverageCie(List<List<SpectroResultStruct>> MeasureList, bool Use120b = false)
        {
            CieLabStruct[] AvgCie = new CieLabStruct[MeasureList.Count];

            for (int StripNo = 0; StripNo < MeasureList.Count; StripNo++)
            {
                AvgCie[StripNo] = new CieLabStruct { L = 0, a = 0, b = 0 };
                for (int MeasureNo = 0; MeasureNo < MeasureList[StripNo].Count; MeasureNo++)
                {
                    if (!Use120b)
                        AvgCie[StripNo].L += MeasureList[StripNo][MeasureNo].CieLab.L;
                    else
                        AvgCie[StripNo].L += (120 - MeasureList[StripNo][MeasureNo].CieLab.b);

                    AvgCie[StripNo].a += MeasureList[StripNo][MeasureNo].CieLab.a;
                    AvgCie[StripNo].b += MeasureList[StripNo][MeasureNo].CieLab.b;
                }
                AvgCie[StripNo].L /= MeasureList[StripNo].Count;
                AvgCie[StripNo].a /= MeasureList[StripNo].Count;
                AvgCie[StripNo].b /= MeasureList[StripNo].Count;
            }

            return AvgCie;
        }

        /// <summary>
        /// Calculates DeltaE for each MeasurePoint against the average of the Density-Strip
        /// </summary>
        /// <param name="MeasureList">Measure Results of current Measurement, MeasureList[DensityStrip][MeasurePoint]SpectroResultStruct</param>
        /// <param name="StripAverage">averages of all L*a*b* for each DensityStrip</param>
        /// <param name="Use120b">Use (120 - b) instead of L* (for Yellow), default = false</param>
        /// <returns>Weighted DeltaE for each Measure-Point in Each Density-Strip, DeList[DensityStrip][MeasurePoint]</returns>
        public List<List<double>> CalcWeightedDeList(List<List<SpectroResultStruct>> MeasureList, CieLabStruct[] StripAverage, bool Use120b = false)
        {
            List<List<double>> DeList = new List<List<double>>();

            for (int StripNo = 0; StripNo < MeasureList.Count; StripNo++)
            {
                List<double> StripDiffList = new List<double>();
                for (int MeasureNo = 0; MeasureNo < MeasureList[StripNo].Count; MeasureNo++)
                {
                    double De = 0;
                    if (!Use120b)
                        De = DeltaE2000(StripAverage[StripNo], MeasureList[StripNo][MeasureNo].CieLab) * WeightList[StripNo];
                    else
                    {
                        CieLabStruct b120 = new CieLabStruct
                        {
                            a = MeasureList[StripNo][MeasureNo].CieLab.a,
                            b = MeasureList[StripNo][MeasureNo].CieLab.b,
                            L = 120 - MeasureList[StripNo][MeasureNo].CieLab.b
                        };
                        De = ColorConversion.DeltaE2000(StripAverage[StripNo], b120) * WeightList[StripNo];
                    }

                    //Add sign to DeltaE
                    double RollAvg = 0;
                    int RollCount = 0;
                    for (int i = ((MeasureNo >= 1) ? (MeasureNo - 1) : MeasureNo); i <= ((MeasureNo < MeasureList[StripNo].Count - 2) ? (MeasureNo + 1) : MeasureNo); i++)
                    {
                        if (!Use120b) RollAvg += MeasureList[StripNo][i].CieLab.L;
                        else RollAvg += (120 - MeasureList[StripNo][i].CieLab.b);
                        RollCount++;
                    }
                    RollAvg /= RollCount;
                    if (StripAverage[StripNo].L > RollAvg) StripDiffList.Add(De * DeSpreadFactor);
                    else StripDiffList.Add(De * -DeSpreadFactor);
                }
                DeList.Add(StripDiffList);
            }

            return DeList;
        }

        /// <summary>
        /// Calculates the average for each Point (12) through all Density-Strips and averages 2 Measure-Points together
        /// </summary>
        /// <param name="WeightedDeList">Weighted DeltaE for each Measure-Point in Each Density-Strip, DeList[DensityStrip][MeasurePoint]DeltaE</param>
        /// <param name="NumHeads">Number of heads</param>
        /// <returns>List with 12 DeltaE values per head, De12List[Head][MeasurePoint]DeltaE</returns>
        public List<List<double>> CalcAverageTo12Points(List<List<double>> WeightedDeList, int NumHeads)
        {
            //Average over 20/30/40/50%
            List<double> AvgDeList = new List<double>();
            for (int PointNo = 0; PointNo < WeightedDeList[0].Count; PointNo++)
            {
                double AvgDe = 0;
                for (int StripNo = 0; StripNo < WeightedDeList.Count; StripNo++)
                {
                    AvgDe += WeightedDeList[StripNo][PointNo];
                }
                AvgDe /= WeightedDeList.Count;
                AvgDeList.Add(AvgDe);
            }

            //Average Measurement to 12 Points per head
            List<List<double>> DeList12 = new List<List<double>>();
            double TotalAverage = 0;
            int TotalCount = 0;
            int PointInHead = 0;
            for (int HeadNum = 0; HeadNum < NumHeads; HeadNum++)
            {
                List<double> HeadDeList12 = new List<double>();
                for (int Point12Num = 0; Point12Num < 12; Point12Num++)
                {
                    double AvgPoint = 0;
                    for (int i = 0; i < 2; i++)
                    {    //First and last Stitch are not measured on Label-Machines
                        if ((HeadNum == 0 && Point12Num == 0) || (HeadNum == NumHeads - 1 && Point12Num >= 11))
                            AvgPoint = 0;
                        else
                        {
                            TotalAverage += AvgDeList[PointInHead];
                            TotalCount++;
                            AvgPoint += AvgDeList[PointInHead++];
                        }
                    }
                    AvgPoint /= 2;
                    HeadDeList12.Add(AvgPoint);
                }
                //Back for overlapping Stitch
                PointInHead -= 2;
                DeList12.Add(HeadDeList12);
            }

            //On Label set first and last Stitch to average
            TotalAverage /= TotalCount;
            DeList12[0][0] = TotalAverage;
            DeList12[NumHeads - 1][11] = TotalAverage;

            return DeList12;
        }

        /// <summary>
        /// Calculates the new Density List
        /// </summary>
        /// <param name="OriginalDensityList">Density List as the current Measurement was printed, 12 Points per head, OriginalDensityList[Head][Point]Density 0-1000</param>
        /// <param name="DeList12">List with 12 DeltaE values per head, De12List[Head][MeasurePoint]DeltaE</param>
        /// <param name="StitchFactors">Array containing correction factors for each stitch area, default = null</param>
        /// <returns>New Density List, 12 Points per head, NewDensityList[Head][Point]Density 0-1000</returns>
        public List<List<int>> CalcNewDensities(int NumHeads, List<List<int>> OriginalDensityList, List<List<double>> DeList12, double[] StitchFactors = null)
        {
            List<List<int>> NewDensityList = new List<List<int>>();
            for (int HeadNum = 0; HeadNum < NumHeads; HeadNum++)
            {
                List<int> NewHeadDensityList = new List<int>();
                for (int PointNum = 0; PointNum < 12; PointNum++)
                {
                    NewHeadDensityList.Add((int)(OriginalDensityList[HeadNum][PointNum] *
                                (1 - ((DeList12[HeadNum][PointNum] / 1000) * DensityCorrectionFactor))));
                    //2020.07.20: best compromise between various drop sizes and various coverages: DensityCorrFactor = 1.42
                }
                NewDensityList.Add(NewHeadDensityList);
            }

            //Apply Stitch-Factors
            if (StitchFactors != null)
            {
                //Left Stitch Area
                for (int HeadNo = 1; HeadNo < NewDensityList.Count; HeadNo++)
                {
                    NewDensityList[HeadNo][0] = (int)(NewDensityList[HeadNo][0] * (1 + (StitchFactors[HeadNo - 1] / 100.0)));
                }
                //right Stitch Area
                for (int HeadNo = 0; HeadNo < NewDensityList.Count - HeadNo; HeadNo++)
                {
                    NewDensityList[HeadNo][11] = (int)(NewDensityList[HeadNo][11] * (1 + (StitchFactors[HeadNo + 0] / 100.0)));
                }
            }

            //Check NewDensityList for Values > 10000
            CheckDensityMax(ref NewDensityList);

            return NewDensityList;
        }

        /// <summary>
        /// Checks and adjusts Density List for Values exceeding 1000
        /// </summary>
        /// <param name="NewDensityList">Density List:[Head][Point]Density</param>
        private void CheckDensityMax(ref List<List<int>> NewDensityList)
        {
            int MaxDens = 0;
            for (int i = 0; i < NewDensityList.Count; i++)
                if (NewDensityList[i].Max() > MaxDens) MaxDens = NewDensityList[i].Max();

            //If Maximum exceeds 1000 adjust all Values by the same factor
            if (MaxDens > 1000)
            {
                double Factor = (double)1000 / (double)MaxDens;
                for (int i = 0; i < NewDensityList.Count; i++)
                {
                    for (int j = 0; j < NewDensityList[i].Count; j++)
                    {
                        NewDensityList[i][j] = (int)(Convert.ToDouble(NewDensityList[i][j]) * Factor);
                    }
                }
            }
        }

        #endregion

        #region Second Run "Gain"

        /// <summary>
        /// Calculates the average L* from current Measure Results and creates a ResultList per Head ([Head][DensityStrip][MeasuePoint]SpectroResultStruct)
        /// </summary>
        /// <param name="MeasureResultList">Measure Results of current Measurement, MeasureResultList[DensityStrip][MeasurePoint]SpectroResultStruct</param>
        /// <param name="CurrentMeasurementL">[Head][DensityStrip][Measue12Point]CieLabStruct</param>
        /// <param name="NumHeads">Number of heads</param>
        /// <param name="Use120b">Use (120 - b) instead of L* (for Yellow), default = false</param>
        /// <returns>average L*</returns>
        private double CalcAverageL(List<List<SpectroResultStruct>> MeasureResultList, ref List<List<List<SpectroResultStruct>>> CurrentMeasurement, int NumHeads, bool Use120b = false)
        {
            //Prepare empty ResultList per Head
            CurrentMeasurement = new List<List<List<SpectroResultStruct>>>();
            for (int HeadNum = 0; HeadNum < NumHeads; HeadNum++)
            {
                List<List<SpectroResultStruct>> StripList = new List<List<SpectroResultStruct>>();
                for (int StripNo = 0; StripNo < MeasureResultList.Count; StripNo++)
                {
                    List<SpectroResultStruct> PointList = new List<SpectroResultStruct>();
                    StripList.Add(PointList);
                }
                CurrentMeasurement.Add(StripList);
            }

            //Fill ResultList per Head
            for (int StripNo = 0; StripNo < MeasureResultList.Count; StripNo++)
            {
                int HeadNo = 0;
                int HeadPointNo = 0;
                for (int PointNo = 0; PointNo < MeasureResultList[StripNo].Count; PointNo++)
                {
                    CurrentMeasurement[HeadNo][StripNo].Add(MeasureResultList[StripNo][PointNo]);

                    //first head has no left stitch, last head has no right stitch
                    HeadPointNo++;
                    if ((HeadNo == 0 && HeadPointNo == 22) || HeadPointNo == 24)
                    {
                        HeadNo++;
                        HeadPointNo = 0;
                        PointNo -= 2;
                    }
                }
            }

            double AvgCurr_L = CalcAverageL(CurrentMeasurement, Use120b);
            return AvgCurr_L;
        }

        /// <summary>
        /// Calculates the average L*
        /// </summary>
        /// <param name="LastMeasurement">[Head][DensityStrip][MeasurePoint]SpectroResultStruct</param>
        /// <param name="Use120b">Use (120 - b) instead of L* (for Yellow), default = false</param>
        /// <returns>average L*</returns>
        private double CalcAverageL(List<List<List<SpectroResultStruct>>> LastMeasurement, bool Use120b = false)
        {
            double AvgCurr_L = 0;
            int PointCount = 0;

            for (int HeadNo = 0; HeadNo < LastMeasurement.Count; HeadNo++)
            {
                for (int StripNo = 0; StripNo < LastMeasurement[HeadNo].Count; StripNo++)
                {
                    for (int PointNo = 0; PointNo < LastMeasurement[HeadNo][StripNo].Count; PointNo++)
                    {
                        if (!Use120b)
                            AvgCurr_L += LastMeasurement[HeadNo][StripNo][PointNo].CieLab.L;
                        else
                            AvgCurr_L += 120 - LastMeasurement[HeadNo][StripNo][PointNo].CieLab.b;
                        PointCount++;
                    }
                }
            }
            AvgCurr_L /= PointCount;

            return AvgCurr_L;
        }

        /// <summary>
        /// Calculates the average gain over all heads between first and last measurement
        /// </summary>
        /// <param name="CurrentDensityList">Density List as the current Measurement was printed, 12 Points per head, OriginalDensityList[Head][Point]Density 0-1000</param>
        /// <param name="LastDensityList">Density List as the first Measurement has been printed, 12 Points per head, OriginalDensityList[Head][Point]Density 0-1000</param>
        /// <param name="CurrentMeasurement"> as [Head][DensityStrip][MeasurePoint]CieLabStruct</param>
        /// <param name="LastMeasurement"> as [Head][DensityStrip][MeasurePoint]CieLabStruct</param>
        /// <param name="Use120b">Use (120 - b) instead of L* (for Yellow), default = false</param>
        /// <returns>average Gain</returns>
        private double CalcAvgGain(List<List<int>> CurrentDensityList, List<List<int>> LastDensityList,
                                   List<List<List<SpectroResultStruct>>> CurrentMeasurement, List<List<List<CieLabStruct>>> LastMeasurement,
                                   bool Use120b = false)
        {
            double AvgGain = 0;
            int PointCount = 0;
            for (int HeadNo = 0; HeadNo < CurrentMeasurement.Count; HeadNo++)
            {
                for (int StripNo = 0; StripNo < CurrentMeasurement[HeadNo].Count; StripNo++)
                {
                    int MeasurePointNo = 0;
                    if (HeadNo > 0) MeasurePointNo = 2;
                    for (; MeasurePointNo < CurrentMeasurement[HeadNo][StripNo].Count; MeasurePointNo++)
                    {
                        int ZonePointNo = MeasurePointNo / 2;

                        if (Math.Abs((CurrentDensityList[HeadNo][ZonePointNo] - LastDensityList[HeadNo][ZonePointNo])) > 10)
                        {
                            if (!Use120b)
                                AvgGain += ((CurrentMeasurement[HeadNo][StripNo][MeasurePointNo].CieLab.L - LastMeasurement[HeadNo][StripNo][MeasurePointNo].L) /
                                            (double)(CurrentDensityList[HeadNo][ZonePointNo] - LastDensityList[HeadNo][ZonePointNo]));
                            else
                                AvgGain += (((120 - CurrentMeasurement[HeadNo][StripNo][MeasurePointNo].CieLab.b) - (120 - LastMeasurement[HeadNo][StripNo][MeasurePointNo].b)) /
                                            (double)(CurrentDensityList[HeadNo][ZonePointNo] - LastDensityList[HeadNo][ZonePointNo]));
                            PointCount++;
                        }
                    }
                }
            }
            if (PointCount > 0) AvgGain /= PointCount;
            if (PointCount == 0 || AvgGain == 0 || double.IsNaN(AvgGain)) AvgGain = 1;

            return AvgGain;
        }

        /// <summary>
        /// Calculates new Density List from Gain between first and last measurement
        /// </summary>
        /// <param name="CurrentDensityList">Density List as the current Measurement was printed, 12 Points per head, OriginalDensityList[Head][Point]Density 0-1000</param>
        /// <param name="CurrentMeasurement"> as [Head][DensityStrip][MeasurePoint]CieLabStruct</param>
        /// <param name="AvgCurr_L">Average current L* or (120 - b*)</param>
        /// <param name="AvgGain">Average Gain</param>
        /// <returns></returns>
        List<List<int>> CalcNewDensitiesGain(List<List<int>> CurrentDensityList, List<List<List<SpectroResultStruct>>> CurrentMeasurement,
                                            double AvgCurr_L, double AvgGain, int NumHeads)
        {
            List<List<int>> NewDensityList = new List<List<int>>();

            //Calculate New Density
            for (int HeadNo = 0; HeadNo < CurrentMeasurement.Count; HeadNo++)
            {
                List<int> NewHeadList = new List<int>();
                //Labelmachines have no measurement for first stitch
                if (HeadNo == 0) NewHeadList.Add(CurrentDensityList[0][0]);

                for (int PointNo = 0; PointNo < CurrentMeasurement[HeadNo][0].Count; PointNo += 2)
                {
                    double PointVal = 0;
                    int ValueCounter = 0;

                    //Average Measure Points over all Density Strips
                    for (int StripNo = 0; StripNo < CurrentMeasurement[HeadNo].Count; StripNo++)
                    {
                        //Two measure Points
                        PointVal += CurrentMeasurement[HeadNo][StripNo][PointNo].CieLab.L;
                        ValueCounter++;
                        PointVal += CurrentMeasurement[HeadNo][StripNo][PointNo + 1].CieLab.L;
                        ValueCounter++;
                    }

                    PointVal /= ValueCounter;

                    //Labelmachines have no measurement for first stitch
                    if (HeadNo == 0)
                        NewHeadList.Add(CurrentDensityList[HeadNo][(PointNo + 2) / 2] + (int)((PointVal - AvgCurr_L) / (-AvgGain)));
                    else
                        NewHeadList.Add(CurrentDensityList[HeadNo][PointNo / 2] + (int)((PointVal - AvgCurr_L) / (-AvgGain)));
                }
                //Labelmachines have no measurement for last stitch
                if (HeadNo == (NumHeads - 1))
                {
                    NewHeadList.Add(999);
                }
                NewDensityList.Add(NewHeadList);
            }

            CheckDensityMax(ref NewDensityList);
            return NewDensityList;
        }


        #endregion
    }
}
