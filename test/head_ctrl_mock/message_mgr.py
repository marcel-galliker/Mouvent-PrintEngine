"""
 Message manager to understand and create message to head_ctrl and FPGA
"""
import struct
import re
import os
import logging

# defines and structs that could be find reading .h
defines = {}
structs = {
    "SCondInfo": "UINT32 info;\n" # union define in cond_def_head.h
}
def read_header(header_dir, header):
    "read .h file and set global dictionaries defines and structs"
    in_struct = False
    # read all lines of the file
    header_file = open(os.path.join(header_dir, header))
    for (n,line) in enumerate(header_file):
        # 
        define = re.match(r"\s*#define\s+(\w+)\s+(.*)", line)
        if define:
            value = re.sub(r"\s*//.*", "", define.group(2))
            value = re.sub(r'(".*"|\w+)\s+((".*"|\w+))',r'\1+\2', value)
            value = re.sub(r'(".*"|\w+)\s+((".*"|\w+))',r'\1+\2', value)
            value = re.sub(r'(0x[0-9a-fA-F]+)UL',r'\1', value)
            if value:
                defines[define.group(1)] = eval(value, defines)
        
        if in_struct:
            ends = re.match(r"}\s*(\w+);", line)
            if ends:
                structs[ends.group(1)] = struct_cont
                in_struct = False
            else:
                struct_cont += line

        bstruct = re.match(r"\s*typedef\s+struct\s*(\w+)?", line)
        if bstruct:
            if in_struct:
                raise struct.error(f"struct in struct line {n} of {header}")
            in_struct = True
            struct_cont = ""
    header_file.close()


message_id = {}
message_name = {}
# all message we can convert and their corresponding C struct from .h file
msgtypes = {
    "CMD_HEAD_STAT": None, # ignore
    "SET_GET_INK_DEF": None,
    "CMD_HEAD_FLUID_CTRL_MODE": None,

    "CMD_RFS_START_PROCESS": None,
    "CMD_RFS_KILL_PROCESS": None,
    "CMD_RFS_MAKE_DIR" : None,
    "CMD_ERROR_RESET" : None,

    "CMD_BOOT_PING": "",
    "CMD_BOOT_INFO_REQ": "",
    "REP_BOOT_INFO":"SNetworkItem",
    "REP_BOOT_PING": "", # will be overwritten by REP_BOOT_INFO
    "CMD_BOOT_ADDR_SET":"SBootAddrSetCmd",
    "REP_HEAD_STAT":"SHeadBoardStat",
    "CMD_PING": "",
    "REP_PING": "",
    "CMD_FPGA_IMAGE": "SFpgaImageCmd",
    "REP_FPGA_IMAGE": "",
    "REP_HEAD_BOARD_CFG":
        "UINT32		resetCnt;",
    "CMD_HEAD_BOARD_CFG": "SHeadBoardCfg",
    "CMD_GET_BLOCK_USED": "SBlockUsedCmd",
    "REP_GET_BLOCK_USED": # special variable struct
    """
        UINT32		aliveCnt[2];
        UINT32		blockOutIdx;	// number of the last block used
        UINT32		blkNo;			// BYTE-index of first used bits
        UINT8		headNo;
        UINT8		id;
        UINT16		blkCnt;				// number of bytes
        UINT32		used[*];// maximum size
    """,
    "EVT_PRINT_DONE": "SPrintDoneMsg",
    "CMD_PRINT_ABORT": "",
}

def cstruct(s):
    "analyse a struct from header to flatenize it and eval define"
    r = ""
    bits = 0
    for l in structs[s].split("\n"): # for all lines of the struct
        # check if it is a field defintion line (else ignore it)
        field = re.match(r"\s*(\w+)\s+(\w+)(\[.*\]|\s*:\s*(\d+))?;", l)
        if field:
            arr = field.group(3)
            typed = field.group(1)
            # remove message header
            if typed == "SMsgHdr" or (typed == "UINT32" and field.group(2)=="id"):
                continue
            if typed.startswith("E"): # Enum = int
                typed = "INT32"
            elif typed.startswith("S"): # the field is a struct 
                # parse the struct
                newstruct=cstruct(typed)
                # and flaten it if possible (not an array)
                if arr:
                    msgtypes[typed] = newstruct
                else:
                    r += newstruct
                    continue
            if arr: # array should be evaludated (define)
                if arr.startswith("["):
                    arr = "[%d]" % eval(arr[1:-1], defines)
            else:
                arr = ""
            if field.group(4): # manage the case of bits table
                bits += int(field.group(4))
                continue
            else:
                while bits > 0:
                    r += f"UINT32 {s.lower()}_{bits};\n"
                    bits -= 32
                bits = 0

            r += typed + " " + field.group(2) + arr +";\n"
    
    while bits > 0: # bit table left?
        r += f"UINT32 {s.lower()}_{bits};\n"
        bits -= 32
    return r

def init(src_path):
    "read the messages configurations in header files and setup needed configuration for struct"
    header_dir = os.path.join(src_path,"rx_common_lib")
    read_header(header_dir, "rx_def.h")
    read_header(header_dir, "fpga_def_head.h")
    read_header(header_dir, "tcp_ip.h")
    # first retrieve the id of all messages from #define in .h
    for m in msgtypes.keys():
        # convert id to name of the message
        message_id[defines[m]] = m
        # and the opposite (name to id)
        message_name[m] = defines[m]
    # then build the corresponding struct
    msgs = list(msgtypes.keys())
    for msg in msgs:
        s = msgtypes[msg]
        if s and s.startswith("S"):
            msgtypes[msg] = cstruct(s)
    # special case BOOT_PING and BOOT info responce are the same
    msgtypes["REP_BOOT_PING"] = msgtypes["REP_BOOT_INFO"]
    

def struct_format(msgtype):
    "convert a message type from its C struct, return (format, names) from the struct of the message"
    if msgtype not in msgtypes:        
        raise struct.error(f"invalid msgtype {msgtype}")

    if msgtypes[msgtype] is None: # ignore this message
        return ("", ())

    names = []
    exformat = ""
    # correpondance between C and python struct 
    cor = {"FLOAT": "f" ,"CHAR": "b", "INT8": "b", "INT16": "h", "INT32": "i", "INT64": "q", "char": "s"}
    # read the C struct representation to make a struct format
    for line in msgtypes[msgtype].split("\n"):
        # if it is a line that defines a member
        var = re.search(r"\s*(U?)(\w+)\s+(\w+)(\[(\d+|\*)\])?;", line)
        if var:
        # then var has 5 groups 
            if var.group(2) not in cor: # special case of struct
                (stformat, stnames) = struct_format(var.group(2))
                sz = 1
                # the 4 and 5 groups (if present) are the size
                if var.group(4):
                    sz = int(var.group(5))
                for _ in range(sz):
                    exformat += stformat
                names.append((var.group(3), sz, stnames))
            else:
                conv = str.lower
                # the first one the  Unsigned
                if var.group(1) == "U":
                    conv = str.upper
                sz = 1
                # the 4 and 5 groups (if present) are the size
                if var.group(4):
                    exformat += var.group(5)
                    if var.group(5) != "*": # various last list (only for pack!)
                        sz = int(var.group(5))
                    else:
                        sz = 99 # does not matter as it is dynamic
                    if var.group(2) == "char": # string are only one member
                        sz = 1
                # the second group is the type of member
                exformat += conv(cor[var.group(2)])
                # and the third is the name of the struct member
                names.append((var.group(3), sz, None))
    return (exformat, names)

def names2dict(names, val, d):
    """convert a (names, val) to a dict
    note than each element of names :
    (0) the member name
    (1) the number of value (1 for single, >1 for list)
    (3) a new list of names in case of nested struct"""
    pos = 0
    for var in names:
        if var[2]: # sub struct
            d[var[0]] = []
            for _ in range(var[1]):
                nd = dict()
                d[var[0]].append(nd)
                pos += names2dict(var[2], val[pos:], nd)
        else:
            if var[1] == 1: # simple
                d[var[0]] = val[pos]
            else: # list
                d[var[0]] = val[pos:pos+var[1]]
            pos += var[1]
    return pos

def dict2name(names, d):
    "return a list of values from dict corresponding to names (see names2dict)"
    val = []
    # all missing data are set to 0
    for var in names:
        if var[2]: # sub struct
            for i in range(var[1]):               
                val.extend(dict2name(var[2], d.get(var[0], var[1] * ({},))[i]))
        elif var[1] == 1: # simple
            val.append(d.get(var[0], 0))
        else: # list
            val.extend(d.get(var[0], var[1] * (0,)))
    return val


class Message:
    "encode / decode message from C struct"
    src_path = os.path.join(os.path.dirname(os.path.abspath(__file__)),"../..")

    def __init__(self, msgtype=None, header="iI"):
        # header is normally (size, msgtype) but could be only msgtype on UDP
        self.header = header
        self.msgtype = msgtype
        # init the messages data the first time so src_path could be overwritten
        if not message_id:
            init(self.src_path)

    def unpack(self, data):
        "create the member of the message from data received"
        # retreive header to know the msgtype
        start = struct.calcsize("="+self.header)
        h = struct.unpack("="+self.header, data[:start])
        self.lenght = len(data)
        # if there are 2 elts in header, the first is the lenght
        if len(self.header) == 2:
            self.lenght = h[0]
        # the last elt of the header is the message type
        if h[-1] in message_id:
            # we know this message
            self.msgtype = message_id[h[-1]]
            # we can unpack the struct
            try:
                (exformat, names) = struct_format(self.msgtype)
                if exformat: 
                    val = struct.unpack("="+exformat, data[start:self.lenght])
                else: # it is a ignored message
                    val = ()
            except struct.error as e:
                logging.error("%s for msg %s (%d bytes)" % (e, self.msgtype, self.lenght))
            else:
                # and retreive in the value the different members
                names2dict(names, val, self.__dict__)
        else:
            self.msgtype = h[-1]
        return data[self.lenght:]

    def pack(self):
        "create the binary message to send"
        (exformat, names) = struct_format(self.msgtype)
        if "*" in exformat: # variable size last list
            exformat = exformat.replace("*", str(len(self.__dict__[names[-1][0]])))
        ft = "=" + self.header + exformat
        val = []
        if len(self.header) == 2: # add the size of message in first pos
            val.append(struct.calcsize(ft))
        val.append(message_name[self.msgtype])
        val.extend(dict2name(names, self.__dict__))
        return struct.pack(ft, *val)

    def __str__(self):
        return(str(self.__dict__))