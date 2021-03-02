"""
Recompose a CMYK image from the printed directory (fake bmp from mock)
"""
from PIL import Image, ImageCms
Image.MAX_IMAGE_PIXELS = None

import xml.etree.ElementTree as ET
import logging
import glob, re

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-conf", help='configuration of the printer', default="d:/radex/user/config.cfg")
    parser.add_argument("-id", help='id of the image', default=1, type=int)
    parser.add_argument("-page", help='page number', default=1, type=int)
    parser.add_argument("-log", help='log level (default INFO)', default="INFO", choices=['DEBUG', 'INFO', 'WARN', 'ERROR'])
    parser.add_argument("-disable", help='disable icc correction on image', action="store_true")
    parser.add_argument("-file", help='name of the file to save')
    parser.add_argument("-v", help='number of virtual passes (2 or 4)',default=1, type=int)
    args = parser.parse_args()

    fn=args.file
    if not fn:
        fn=f"id{args.id}p{args.page}.png"


    numeric_level = getattr(logging, args.log.upper(), None)

    if not isinstance(numeric_level, int):
        raise ValueError('Invalid log level: %s' % args.log)
    logging.basicConfig(level=numeric_level)

    # read the configuration of the printer to retrieve color offsets
    root = ET.parse(args.conf).getroot()
    printer = int(root.find("Printer").attrib["type"])
    logging.debug(f"printer {printer}")
    coloroffset = []
    head_color = int(root.attrib["HeadsPerColor"])
    colors = []
    convcol = {"Black": "K", "Cyan": "C", "Magenta": "M", "Yellow": "Y"}
    for ink in root.iter('InkSupply'):
        if ink.attrib["InkName"]:
            for c in convcol:
                if c in ink.attrib["InkName"]:
                    colors.append(convcol[c])
                    break
            else:
                logging.error( "color unknown " + ink.attrib["InkName"])
        else:
            colors.append(None)
        coloroffset.append(int(ink.attrib["ColorOffset"]) * 1200 // 25400) # offset in pixels for each color
    logging.debug(f"colors : {colors}")

    imgc = [] # images of "current" head by color resize on the same size
   
    # compute size
    incr = 2048 // args.v # size of a head in pixel
    if printer >= 2000 and printer < 3000: # TX printer
        w = incr * (len(glob.glob(f"printed/fake id{args.id} c* s* b* h*.bmp"))//4 - len([x is None for x in colors])) 
    else:
        w = head_color * incr

    # retreive fake bmp by color (depending on number of heads by color and colors order)
    bmps= {} # fake ordered bmp by color
    img = None # full image that will be built head by head / scan by scan
    pos = 4 * [0,] # the current printed position by color
    for (i, color) in enumerate("CMYK"):
        order = colors.index(color)
        (board, head) = divmod(order * head_color, 4)
        lboard = board + (head_color + 3) // 4 - 1
        copy ="*" if printer >= 2000 else "1" # only the first copy on LB
        file_format = f"printed/fake id{args.id} c{copy} p{args.page} s* b[{board}-{lboard}] h[{head}-{head + head_color - 1}].bmp"
        logging.debug(f"search files for {file_format} for color {color}")
        bmps[color] = glob.glob(file_format)
        if not bmps[color]:
            raise KeyError(f"no files for color {color} (board {board}, head {head}-{head + head_color - 1})")
        bmps[color].sort(key=lambda f:[int(x) for x in re.findall(r"(?<=[sb])\d+", f)]) # sort by scan and by board
        logging.debug(f"sorted files {bmps[color]}")
        # start from the begining of printing of the color (for TX)
        if printer >= 2000:
            while pos[i] + head_color * incr - 128 < coloroffset[order]:
                pos[i] += head_color * incr - 128
                del bmps[color][:head_color]
  
    # first we should align the part of the image printed by the different colors
    # according to head positions and color offsets
    imgc = {} # aligned images by color
    x = 0 # abscissa where to paste the next "printed" image
    h = 1 # heigh of the final image
    overlap = 128 if args.v == 1 else 128 + head_color *(2048 + 128) // args.v
    # for each existing scam/head 
    while any(bmps.values()):
        width = 9999 # big value
        offset = 0
        # retreive the current image of each color
        for (nb, color) in enumerate("CMYK"):
            logging.debug(f"color {color}")
            order = colors.index(color)
            if bmps[color]:
                bmp = bmps[color].pop(0)
                i = Image.open(bmp)
                logging.debug(f"size of {bmp}: {i.size}")
                if i.size != (1, 1):
                    h = max(h, i.height)
                    start = 0 #128 # stitching size, so already "print" by previous head
                    if color not in imgc: # first scan (or head)
                        logging.debug(f"new color")
                        if printer >= 2000: # TX
                            start = coloroffset[order] - pos[nb] + start
                        else:
                            start = pos[nb] - coloroffset[order]
                        if start < 0:
                            offset = max(offset, -start) # "negatif" margin on the final image
                    logging.debug(f"start {start}")
                    # check that we should print this color (for scanning on TX, always true on LB)
                    if start < incr:
                        i = i.crop((start, 0, i.width, i.height))
                        if color not in imgc:
                            imgc[color] = i
                        else:
                            # we could have some remaining part of the previous head (color offset)
                            ni = Image.new("L", (imgc[color].width + i.width,i.height))
                            ni.paste(imgc[color], (0,0))
                            ni.paste(i, (imgc[color].width,0), i)
                            imgc[color] = ni
                        logging.debug(f"width {imgc[color].width}")
                        width = min(imgc[color].width, width)
                    else:
                        width = 0
            else: # no more image for the color, use the remaining
                width = min(imgc[color].width, width)

            pos[nb] += incr # next scan (for TX)

        if width != 0 and width < 9999:
            cmyk = [] # combinated image in CMYK order
            for color in "CMYK":
                # resize all color image to the same size
                cmyk.append(imgc[color].crop((offset,0,width,h)))
                # remain of the head that is not yet "printed"
                imgc[color] = imgc[color].crop((width,0,imgc[color].width,h))

            # recreate the full colored CMYK image printed by the "current" heads
            width -= offset
            b = b"".join([bytes(x) for x in zip(*(b.tobytes() for b in cmyk))])
            logging.debug(f"Position {x}: {width}x{h}")
            ip = Image.frombuffer("CMYK", (width,h), b, "raw", "CMYK;I")
            if img is None:
                img = Image.new("CMYK", (w,h), (0,0,0,0))
            # and append it to what was printed so far
            img.paste(ip, (x, 0))
            x += width

    # and finally (optionnal as it is only for better color display) apply an ICC profile
    if not args.disable:
        logging.debug("correct image with standard SWOP ICC profile")
        img = ImageCms.profileToProfile(img, r'C:\Program Files\Mouvent DFE\Mouvent_App\Server\USWebCoatedSWOP.icc', r'C:\Windows\System32\spool\drivers\color\sRGB Color Space Profile.icm', renderingIntent=0, outputMode='RGB')
    
    # and save the full colors image
    img.save(fn)
    logging.info(f"file {fn} saved")
