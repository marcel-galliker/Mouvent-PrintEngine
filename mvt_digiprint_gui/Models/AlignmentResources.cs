using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace RX_DigiPrint.Models
{
    
    public class BitmapInformation
    {
        public System.Windows.Media.ImageSource ImageSource { get; set; }
        public string ImageHeader { get; set; }
    }
    
    public class AlignmentResources
    {
        public enum AlignmentCorrectionType
        {
            TypeUndefined,
            TypePositiveCorrectionValue,
            TypeNegativeCorrectionValue,
        }

        public enum MachineType
        {
            MachineTypeUndef,
            MachineTypeNonScanning,
            MachineTypeScanning,
        }
        
        public Dictionary<AlignmentCorrectionType, List<BitmapInformation>> AngleCorrectionBitmaps { get; set; }
        public Dictionary<AlignmentCorrectionType, System.Windows.Media.ImageSource> AngleFaultTypeSelectionBitmap { get; set; }
        public Dictionary<AlignmentCorrectionType, List<BitmapInformation>> StitchCorrectionBitmaps { get; set; }
        public Dictionary<AlignmentCorrectionType, System.Windows.Media.ImageSource> StitchFaultTypeSelectionBitmap { get; set; }
        
        public Dictionary<AlignmentCorrectionType, System.Windows.Media.ImageSource> RegisterFaultTypeSelectionBitmap { get; set; }
        public Dictionary<AlignmentCorrectionType, System.Windows.Media.ImageSource> BackwardRegisterFaultTypeSelectionBitmap { get; set; }
        public Dictionary<MachineType, Dictionary<AlignmentCorrectionType, System.Windows.Media.ImageSource>> ColorOffsetFaultTypeSelectionBitmap { get; set; }
        
        public AlignmentResources()
        {
            
            AngleCorrectionBitmaps = new Dictionary<AlignmentCorrectionType, List<BitmapInformation>>();
            AngleCorrectionBitmaps[AlignmentCorrectionType.TypePositiveCorrectionValue] = new List<BitmapInformation> {
                new BitmapInformation
                { ImageSource = LoadImage("/Resources/Bitmaps/Alignment/AngleAdjustmentLeftStep1.png"),
                    ImageHeader = "Start Position"},
                new BitmapInformation { ImageSource = LoadImage("/Resources/Bitmaps/Alignment/AngleAdjustmentLeftStep2.png"),
                    ImageHeader = "Step 1: Move Nut down until stops"},
                new BitmapInformation { ImageSource = LoadImage("/Resources/Bitmaps/Alignment/AngleAdjustmentLeftStep3.png"),
                    ImageHeader = "Step 2: Move Nut up until in Final Position"},
                new BitmapInformation { ImageSource = LoadImage("/Resources/Bitmaps/Alignment/AngleAdjustmentLeftStep4.png"),
                    ImageHeader = "Final Position"},
            };
            AngleCorrectionBitmaps[AlignmentCorrectionType.TypeNegativeCorrectionValue] = new List<BitmapInformation>
            {
                new BitmapInformation { ImageSource = LoadImage("/Resources/Bitmaps/Alignment/AngleAdjustmentRightStep1.png"),
                         ImageHeader = "Move Nut up to Final Position"},
                new BitmapInformation { ImageSource = LoadImage("/Resources/Bitmaps/Alignment/AngleAdjustmentRightStep2.png"),
                         ImageHeader = "Final Position"},
            };
            AngleFaultTypeSelectionBitmap = new Dictionary<AlignmentCorrectionType, System.Windows.Media.ImageSource>();
            AngleFaultTypeSelectionBitmap[AlignmentCorrectionType.TypePositiveCorrectionValue] = LoadImage("/Resources/Bitmaps/Alignment/AngleLeft.png");
            AngleFaultTypeSelectionBitmap[AlignmentCorrectionType.TypeNegativeCorrectionValue] = LoadImage("/Resources/Bitmaps/Alignment/AngleRight.png");

            StitchCorrectionBitmaps = new Dictionary<AlignmentCorrectionType, List<BitmapInformation>>();
            StitchCorrectionBitmaps[AlignmentCorrectionType.TypePositiveCorrectionValue] = new List<BitmapInformation> {
                new BitmapInformation
                { ImageSource = LoadImage("/Resources/Bitmaps/Alignment/StitchAdjustmentLeftStep1.png"),
                    ImageHeader = "Move Wedge down to Final Position"},
                new BitmapInformation { ImageSource = LoadImage("/Resources/Bitmaps/Alignment/StitchAdjustmentLeftStep2.png"),
                    ImageHeader = "Final Position"},
            };

            StitchCorrectionBitmaps[AlignmentCorrectionType.TypeNegativeCorrectionValue] = new List<BitmapInformation> {
                new BitmapInformation
                { ImageSource = LoadImage("/Resources/Bitmaps/Alignment/StitchAdjustmentRightStep1.png"),
                    ImageHeader = "Start Position"},
                new BitmapInformation { ImageSource = LoadImage("/Resources/Bitmaps/Alignment/StitchAdjustmentRightStep2.png"),
                    ImageHeader = "Step 1: Move Wedge up until stops"},
                new BitmapInformation
                { ImageSource = LoadImage("/Resources/Bitmaps/Alignment/StitchAdjustmentRightStep3.png"),
                    ImageHeader = "Step 2: Move Wedge down until in Final Position"},
                new BitmapInformation { ImageSource = LoadImage("/Resources/Bitmaps/Alignment/StitchAdjustmentRightStep4.png"),
                    ImageHeader = "Final Position"},
            };
            StitchFaultTypeSelectionBitmap = new Dictionary<AlignmentCorrectionType, System.Windows.Media.ImageSource>();
            StitchFaultTypeSelectionBitmap[AlignmentCorrectionType.TypePositiveCorrectionValue] = LoadImage("/Resources/Bitmaps/Alignment/StitchLeft.png");
            StitchFaultTypeSelectionBitmap[AlignmentCorrectionType.TypeNegativeCorrectionValue] = LoadImage("/Resources/Bitmaps/Alignment/StitchRight.png");

            RegisterFaultTypeSelectionBitmap = new Dictionary<AlignmentCorrectionType, System.Windows.Media.ImageSource>();
            RegisterFaultTypeSelectionBitmap[AlignmentCorrectionType.TypePositiveCorrectionValue] = LoadImage("/Resources/Bitmaps/Alignment/SoftwareValueCorrection1.png");
            RegisterFaultTypeSelectionBitmap[AlignmentCorrectionType.TypeNegativeCorrectionValue] = LoadImage("/Resources/Bitmaps/Alignment/SoftwareValueCorrection2.png");

            BackwardRegisterFaultTypeSelectionBitmap = new Dictionary<AlignmentCorrectionType, System.Windows.Media.ImageSource>();
            BackwardRegisterFaultTypeSelectionBitmap[AlignmentCorrectionType.TypePositiveCorrectionValue] = LoadImage("/Resources/Bitmaps/Alignment/SoftwareValueCorrection1.png");
            BackwardRegisterFaultTypeSelectionBitmap[AlignmentCorrectionType.TypeNegativeCorrectionValue] = LoadImage("/Resources/Bitmaps/Alignment/SoftwareValueCorrection2.png");

            ColorOffsetFaultTypeSelectionBitmap = new Dictionary<MachineType, Dictionary<AlignmentCorrectionType, System.Windows.Media.ImageSource>>();
            ColorOffsetFaultTypeSelectionBitmap[MachineType.MachineTypeNonScanning] = new Dictionary<AlignmentCorrectionType, System.Windows.Media.ImageSource>();
            ColorOffsetFaultTypeSelectionBitmap[MachineType.MachineTypeScanning] = new Dictionary<AlignmentCorrectionType, System.Windows.Media.ImageSource>();
            ColorOffsetFaultTypeSelectionBitmap[MachineType.MachineTypeNonScanning][AlignmentCorrectionType.TypePositiveCorrectionValue] = LoadImage("/Resources/Bitmaps/Alignment/SoftwareValueCorrection1.png");
            ColorOffsetFaultTypeSelectionBitmap[MachineType.MachineTypeNonScanning][AlignmentCorrectionType.TypeNegativeCorrectionValue] = LoadImage("/Resources/Bitmaps/Alignment/SoftwareValueCorrection2.png");
            ColorOffsetFaultTypeSelectionBitmap[MachineType.MachineTypeScanning][AlignmentCorrectionType.TypePositiveCorrectionValue] = LoadImage("/Resources/Bitmaps/Alignment/SoftwareValueCorrection3.png");
            ColorOffsetFaultTypeSelectionBitmap[MachineType.MachineTypeScanning][AlignmentCorrectionType.TypeNegativeCorrectionValue] = LoadImage("/Resources/Bitmaps/Alignment/SoftwareValueCorrection4.png");
        }
        
        private System.Windows.Media.ImageSource LoadImage(string imagePath)
        {
            System.Windows.Media.Imaging.BitmapImage bitmapImage = null;
            try
            {
                bitmapImage = new System.Windows.Media.Imaging.BitmapImage();
                bitmapImage.BeginInit();
                bitmapImage.CacheOption = System.Windows.Media.Imaging.BitmapCacheOption.OnLoad;
            
                bitmapImage.UriSource = new Uri("pack://application:,,," + imagePath);
            }
            catch (Exception e)
            {

                Console.WriteLine(e.Message);
                return null;
            }
            try
            {
                bitmapImage.EndInit();
                bitmapImage.Freeze();

            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                return null;
            }
            return bitmapImage;
            
        }
        
    }
}
