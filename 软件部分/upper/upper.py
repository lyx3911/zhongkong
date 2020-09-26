import cv2
import numpy as np
import warnings
import os
from skimage import io, transform
import tensorflow as tf
import serial
import time
import globalvariables as gl

warnings.filterwarnings("ignore")
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'

w = 100
h = 100
c = 3
port = "COM7"
rate = 9600
#初始化串口
ser = serial.Serial(port, rate, timeout = 0.5)
time.sleep(10)

def get_pic():
    #打开摄像头获取图片

    pic = cv2.VideoCapture(1)

    ret, frame = pic.read()
    img2 = frame[60: 420, 0: 640]
    cv2.imshow("pic", img2)
    img2 = transform.resize(img2,(w,h))
    
    #cv2.waitKey(0)
    pic.release()
    cv2.destroyAllWindows()

    return np.asarray(img2)

def predict():
    #预测物品种类
    with tf.Session() as sess:
        data = []
        data1 = get_pic()

        data.append(data1)
     
        saver = tf.train.import_meta_graph('./model.ckpt.meta')
        saver.restore(sess, tf.train.latest_checkpoint('./'))

        graph = tf.get_default_graph()
        x = graph.get_tensor_by_name("x:0")
        feed_dict = {x: data}

        logits = graph.get_tensor_by_name("logits_eval:0")

        classification_result = sess.run(logits, feed_dict)

        output = []
        output = tf.argmax(classification_result, 1).eval()

        # for i in range(len(output)):
        #     print("第", i + 1, "个预测:" + str(output[i]+1))
        print("预测:" + str(output[0] + 1))
        with open('predict_result.txt', 'w') as f:
            f.write(str(output[0]+1))

        print('Python Done')
        return output[0] + 1

def control_plc(line):
    #给下位机发送指令
    print('#send ' + line)
    ser.write(line.encode())
    
    tmp = ser.readline()
    while len(tmp) == 0:
        tmp = ser.readline()
    
    print(tmp)
    return tmp

def move(dis):
    print('#move '+str(dis))
    control_plc('m' + str(dis))
    
    gl.curx += gl.dx[gl.curDir] * dis
    gl.cury += gl.dy[gl.curDir] * dis
    print(gl.curx, ' ', gl.cury, ' ', gl.curDir)

def turnTo(newDir):
    print('#turnTo '+str(newDir))
    control_plc('t' + str(newDir))

    gl.curDir = newDir
    print(gl.curx, ' ', gl.cury, ' ', gl.curDir)

def turn(arg):
    print('#turn ' + str(arg))
    control_plc('f' + str(arg))

    gl.curDir = (gl.curDir + 4 + arg * 2 - 1) % 4
    print(gl.curx, ' ', gl.cury, ' ', gl.curDir)
    

def reconstructRoute(stat):
    x, y, d = stat
    #print('#recon ' + str(x) + ' ' + str(y) + ' ' + str(d))
    if gl.pre[stat] == stat:
        return
    prex, prey, pred = gl.pre[stat]
    reconstructRoute((prex, prey, pred))
    if pred != d:
        turn((3 - (d - pred + 4) % 4) // 2)
    
    move(abs(x-prex + y-prey))

def bfs(tgtStat):
    gl.pre = {}
    vis = {}
    q = []

    stat = gl.curx, gl.cury, gl.curDir
    q.append(stat)
    vis[stat] = True
    gl.pre[stat] = stat

    while q:
        stat = (x, y, dir) = q.pop(0)

        if stat == tgtStat:
            break

        tx, ty = x, y
        while gl.map[tx+gl.dx[dir], ty+gl.dy[dir]]:
            tx += gl.dx[dir]
            ty += gl.dy[dir]
            tStat = tx, ty, dir
            if tStat not in vis:
                vis[tStat] = True
                q.append(tStat)
                gl.pre[tStat] = stat
        
        if gl.turnFree[x, y]:
            for i in range(4):
                if (i + dir) % 2 == 1:
                    tStat = x+gl.dx[i], y+gl.dy[i], i
                    if gl.map[tStat[0], tStat[1]] and tStat not in vis:
                        vis[tStat] = True
                        gl.pre[tStat] = stat
                        q.append(tStat)

    if tgtStat in vis:
        reconstructRoute(tgtStat)
        print('bfs complete')
        return True
    else:
        print('bfs failed')
        return False

def detectBarrier():
    for i in range(5):
        bfs(gl.barrierDetectStat[i])

        if control_plc('d')== b'0\r\n':

            gl.map[gl.barrierPos[i]] = True

            for j in range(4):
                tx, ty = gl.barrierPos[i][0] + gl.dx[j], gl.barrierPos[i][1] + gl.dy[j]
                if (tx > 1 and tx < 10 and ty > 1 and ty < 10) or (tx, ty) in gl.turnFreeExecption:
                    gl.turnFree[tx, ty] = True

        else:
            for k in range(i+1, 6):
                gl.map[gl.barrierPos[k]] = True

                for j in range(4):
                    tx, ty = gl.barrierPos[k][0] + gl.dx[j], gl.barrierPos[k][1] + gl.dy[j]
                    if (tx > 1 and tx < 10 and ty > 1 and ty < 10) or (tx, ty) in gl.turnFreeExecption:
                        gl.turnFree[tx, ty] = True
            return i
    return 5

def findEmptyShelfAndPut(id):
    for i in range(gl.shelfStart[id], 6):
        if bfs(gl.shelfStat[id][i]):
            gl.shelfStart[id] = i
            ret = control_plc('e')

            if ret == b'0\r\n' or ret == b'2\r\n':
                control_plc('c1')
                break
            elif ret == b'1\r\n':
                control_plc('c0')
                break
            elif i == 5:
                bfs(gl.dumpStat[id])
                control_plc('c0')
                break
        elif i == 5:
            bfs(gl.dumpStat[id])
            control_plc('c0')

'''
barrierPos = 6#detectBarrier()
nogoZone = barrierPos // 2 + 1
print('barrier detection complete')
'''
