import numpy as np
import cv2
import matplotlib.pyplot as plt
import sys


HORIZONTAL = 0
VERTICAL = 1

a = [179,181,180,185,183,143,90,100,158,182,181,184,183]

#------------------------------------------------------------
#de una lista de valores calcula la media movil(3)
def process5(lista):
    result = []
    for i in range(len(lista)):
        val = 0
        if i == len(lista)-4:
            val = (int(lista[i])+int(lista[i+1])+int(lista[i+2])+int(lista[i+3]))/4
        elif i == len(lista)-3:
            val = (int(lista[i])+int(lista[i+1])+int(lista[i+2]))/3
        elif i == len(lista)-2:
            val = (int(lista[i])+int(lista[i+1]))/2
        elif i == len(lista)-1:
            val = lista[i]
        else:
            val = (int(lista[i])+int(lista[i+1])+int(lista[i+2])+int(lista[i+3])+int(lista[i+4]))/5
        result.append(val)

    return result

def process2(lista):
    result = []
    for i in range(len(lista)):
        val = 0
        if i == len(lista)-1:
            val = lista[i]
        else:
            val = (int(lista[i])+int(lista[i+1]))/2
        result.append(val)
    return result

#esta parte es para eliminar pequenas variaciones
#trabajamos con una media movil de 3, si aumenta se tiene mucha mayor atenuacion y si se disminuye
#tenemos menor pooder de filtrado, ya que esta actuando como un filtro paso alto detecta variaciones de intensidad
#ha cierta frecuencia

def process(lista):
    result = []
    for i in range(len(lista)):
        val = 0
        '''
        if i == len(lista)-4:
            val = (lista[i]+lista[i+1]+lista[i+2]+lista[i+3])/4
        elif i == len(lista)-3:
            val = (lista[i]+lista[i+1]+lista[i+2])/3
        '''
        if i == len(lista)-2:
            val = (int(lista[i])+int(lista[i+1]))/2
        elif i == len(lista)-1:
            val = int(lista[i])
        else:
            val = (int(lista[i])+int(lista[i+1])+int(lista[i+2]))/3
            #if i == 0:
             #   print(lista[i]+lista[i+1]+lista[i+2])
        result.append(val)

    return result
    
def resta(list1,list2):
    assert len(list1) == len(list2)
    c = [list1[i]-list2[i] for i in range(len(list1))]
    return c

#dibuja una lista de numeros
def draw_lista(lista):
    num_plots = len(lista)
    rows = cols = int(num_plots**0.5)
    rows = rows if rows**2 == num_plots else rows+1

    plt.figure(1)
    x_dim = len(lista[0])
    x = np.arange(0,x_dim,1)
    for i,signal in enumerate(lista):
        plt.subplot(rows,cols,i+1)
        plt.plot(x,signal)
        plt.title(str(i))

#draw by row and col
def draw_lista_row_col(lista,img):
    img_copy =  img.copy()
    for row,data in enumerate(lista):
        for tupla in data:
            x1,x2 = tupla[0],tupla[1] 
            cv2.line(img_copy,(x1,row),(x2,row),(255,0,0),1)
    show(img_copy)
    
#dibuja una linea en una imagen (una fila)
def draw(row,img):
    img_copy = img.copy()
    cv2.line(img_copy,(0,row),(img_copy.shape[1]-1,row),(255,0,0),1)
    show(img_copy)

def drawSeveral(lista,img):
    img_copy = img.copy()
    for row in lista:
        cv2.line(img_copy,(0,row),(img_copy.shape[1]-1,row),(255,0,0),1)
    show(img_copy)

#primer filtrado sera por amplitud para este caso es 35


def amplitud(row):
    return [i if i>35 or i<-35 else 0 for i in row]
def filter1(img):
    media = map(process,img)
    final = map(lambda x,y: map(lambda a,b:a-b, x, y), img, media)
    delete = map(amplitud, final)
    return delete
    #media = [process(row) for row in img]
    #final = [resta(media[i],img[i]) for i in range(media)]


#encontramos variaciones intensidad ..contraste, las secuencias tienes variaciones de blanco a negro
# y de negro a blanco con cierta frecuencia
def findVariations(row):
    lgt = len(row)
    start,end,cuenta = 0,0,0
    secuencias = []
    for i,px in enumerate(row):
        if  px == 0:
            cuenta += 1
        else:
            cuenta = 0
            if start == end:
                start,end = i if i == 0 else (i-1),i
            else:
                end = i
        if cuenta > 15:
            if not start == end:
                secuencias.append((start,end if end ==(lgt-1) else (end+1)))
                start = end
    # sino son iguales so falta contar una secuencia
    if not start == end:
        secuencias.append((start,end if end==(lgt-1) else (end+1)))

    return secuencias

#recibe una lista de tuplas
def deleteNoise(lista):
    result = []
    for tupla in lista:
        #es considerado como un bloque > 25px
        if tupla[1] - tupla[0] >= 25:
            result.append(tupla)
    return result

def filterWithCharacters(lista,row):
    result = []
    for tupla in lista:
        caracters = countCharacters(row[tupla[0]:tupla[1]+1])
        #print (tupla,caracters)
        if caracters >= 2:
            result.append(tupla)
    return result

#contamos en mumero de caracteres en un bloque un caracter es pico+ y pico-
#si hay dos ciclos juntos entonces asumimos que es un mismo caracter
#un caracter como minimo tiene un ciclo y como maximo dos ciclos

def countCharacters(row):
    ciclos = 0
    px_last = 0
    status = 1
    new_medio_char, characters, cuenta = False, 0, 0
    
    for px_actual in row:
        if px_actual==px_last and px_actual==0 and status == 1:
            status = 1
            if new_medio_char: cuenta += 1
            #sys.stdout.write(str(status))
        if px_actual>px_last and px_actual>0 and status==1:
            status = 2
            #print(status)
        if px_actual<px_last and px_actual>0 and status==2:
            status = 3
            #print(status)
        if px_actual<px_last and px_actual==0 and (status==3 or status==2):
            status = 4
            #print(status)
        if px_actual<px_last and px_actual<0 and (status==2 or status==3 or status==4):
            status = 5
        
        if px_actual==px_last and px_actual ==0 and status==4:
            #transicion pico+ to pico-
            pass
            
        if px_actual>px_last and px_actual<0 and status==5:
            status = 6
            #print(status)
        if px_actual>px_last and px_actual>=0 and (status== 6 or status==5):
            status = 1 if px_actual == 0 else 2
            ciclos += 1
            #print(cuenta,new_medio_char)
            if cuenta > 5:
                characters += 1
                new_medio_char = True
            elif cuenta <= 5 and new_medio_char:
                characters += 1
                new_medio_char = False
            elif cuenta <= 5 and not new_medio_char:
                new_medio_char = True
            cuenta = 0
            
        px_last = px_actual

    if new_medio_char: characters += 1
    return characters

#deletenoise2 primero elimina las tuplas(en un row donde comienza y termina una posible secuencia),
#que estan solas y que tiene una amplitud pequena
#ahora en secuencias muy grandes un caracter puede estar aislado de un bloque entonces este caracter
#se lo clasifica como un minibloque entonces vemos si tiene bloques adyacente (por la derecha o izquierda)
#si es que hay y si hay una relacion entre posibles carateres en el bloque y la amplitud entonces se une el minibloque
#al bloque
 

def deleteNoise2(lista,row):        
    def is_to_merge(adyacente,tupla):
        pixeles = adyacente[1]-adyacente[0]
        characters = countCharacters(row[adyacente[0]:adyacente[1]+1])
        #print('characteres',characters,tupla)
        if characters == 0: return False
        r = pixeles/characters
        #print('r',r)
        distancia = tupla[0]-adyacente[1] if (tupla[0]-adyacente[1])>0 else adyacente[0]-tupla[1]
        #print('distancia',distancia)
        return True if distancia < (r+r*25/100) else False

    result = []
    for i,tupla in enumerate(lista):
        #primero vemos si es que es un bloque o minibloque
        #sys.stdout.write(str(i))
        if tupla[1] - tupla[0] >= 25:
            result.append(tupla)
        elif tupla[1]-tupla[0]<5 or len(lista)==1:
            continue
        elif i ==0:
            #vemos si los adyacentes son bloques y no minibloques
            sgt = lista[i+1]
            if sgt[1]-sgt[0] >= 25:
                if is_to_merge(lista[i+1],tupla):
                    result.append((tupla[0],tupla[1],1))
        elif i==len(lista)-1:
            prev = lista[i-1]
            if prev[1]-prev[0] >= 25:
                if is_to_merge(lista[i-1],tupla):
                    result.append((tupla[0],tupla[1],0))

        else:
            prev = lista[i-1]
            if prev[1]-prev[0] >= 25:
                if is_to_merge(prev,tupla):
                    result.append((tupla[0],tupla[1],0))
                    continue
            sgt = lista[i+1]
            if sgt[1]-sgt[0] >= 25:
                if is_to_merge(sgt,tupla):
                    result.append((tupla[0],tupla[1],1))

    return result
            
def mergeTuplas(lista):
    result = []
    flag = False
    #print(lista)
    for i,tupla in enumerate(lista):
        if len(tupla) == 3:
            if tupla[2] == 0:
                aux = result.pop()
                new = (aux[0],tupla[1])
                result.append(new)
            else:
                flag = True
        elif flag:
            flag = False
            aux = lista[i-1]
            new = (aux[0],tupla[1])
            result.append(new)
        else:
            result.append(tupla)

    return result

#dejaremos son las lineas que tienen un grosor en comun
#haciendo refererencia a que las letras de las secuencias tiene un grosor en comun
#B-N grosor N-B(rise grosor fall)
#tuplas contiene los indices donde hay una secuencia en el row
def filterGrosor(tuplas,row):
    pass


#se hara una verficacion vertical para filtrar las lineas que no
#corresponden a una secuencia
#azul contine una lista de lista de tuplas, cada elemento es una lista
#de tuplas que hace referencia a un row de la imagen
#si la sgte row no tiene tuplas en tu rango entonces eres delete
def isSolapado(tupla,row):
    for test in row:
        x1, x2 = test[0], test[1]
        y1, y2 = tupla[0], tupla[1]
        if (x1 >= y1 and x1 <= y2) or (x2 >= y1 and x2 <= y2):
            return True
    return False

def filterY(azul):
    for i,row in enumerate(azul):
        for tupla in row:
            if i == 0:
                if isSolapado(tupla, azul[i+1]): continue
                azul[i].remove(tupla)

            elif i == len(azul)-1:
                if isSolapado(tupla, azul[i-1]): continue
                azul[i].remove(tupla)

            else:
                if isSolapado(tupla, azul[i+1]): continue
                if isSolapado(tupla, azul[i-1]): continue
                azul[i].remove(tupla)
                
    return azul

def filterY(azul):
    for i,row in enumerate(azul):
        
        for tupla in row:
            if i == 0:
                if isSolapado(tupla, azul[i+1]): continue
                azul[i].remove(tupla)

            elif i == len(azul)-1:
                if isSolapado(tupla, azul[i-1]): continue
                azul[i].remove(tupla)

            else:
                if isSolapado(tupla, azul[i+1]): continue
                if isSolapado(tupla, azul[i-1]): continue
                azul[i].remove(tupla)
                
    return azul

def isPlate(widht,height):
    return  widht >= 2*height and widht <= 4*height
    
        

#-------------------------------------------------------
#2do metodo






#-------------------------------------------------------

    
#------------------------------------------------------------------------
#muestra una imagen en sub-ventanas y su media scala de grises

def show(img):
    cv2.imshow('img',img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

def split_image(img_src):
    
    width = 800
    height = 450
    x = 40
    y = 25
    img = img_src.copy()

    for i in range(width/x):
        cv2.line(img,(i*x,0),(i*x,img.shape[0]-1),(255,0,0),1)
    for i in range(height/y):
        cv2.line(img,(0,i*y),(img.shape[1]-1,i*y),(255,0,0),1)

    for i in range(height/y):
        for j in range(width/x):
            point_x = j*x
            point_y = i*y
            #calcular la media de cada box
            box = img_src[point_y:point_y+y,point_x:point_x+x]
            almacen = 0
            for row in range(box.shape[0]):
                for col in range(box.shape[1]):
                    almacen += box[row,col]
            value  = almacen/(box.shape[0]*box.shape[1])
            
            
            cv2.putText(img,str(value),(point_x,point_y+y/2),cv2.FONT_HERSHEY_PLAIN,1,(0,255,0),2)

    show(img)

#-------------------------------------------------------------------------
    
    
    
    
    
def countCharacters2(row):
    ciclos = 0
    px_last = 0
    status = 1
    new_medio_char, characters, cuenta = False, 0, 0
    grosor = 0
    lista_grosor = []
    
    for px_actual in row:
        if px_actual==px_last and px_actual==0 and status == 1:
            status = 1
            if new_medio_char: cuenta += 1
            #sys.stdout.write(str(status))
        if px_actual>px_last and px_actual>0 and status==1:
            status = 2
            #print(status)
        if px_actual<px_last and px_actual>0 and status==2:
            status = 3
            #print(status)
        if px_actual<px_last and px_actual==0 and (status==3 or status==2):
            status = 4
            #print(status)
        if px_actual<px_last and px_actual<0 and (status==2 or status==3 or status==4):
            status = 5
        
        if px_actual==px_last and px_actual ==0 and status==4:
            #transicion pico+ to pico-
            grosor += 1
            
        if px_actual>px_last and px_actual<0 and status==5:
            status = 6
            #print(status)
        if px_actual>px_last and px_actual>=0 and (status== 6 or status==5):
            status = 1 if px_actual == 0 else 2
            ciclos += 1
            lista_grosor.append(grosor)
            grosor = 0
            #print(cuenta,new_medio_char)
            if cuenta > 5:
                characters += 1
                new_medio_char = True
            elif cuenta <= 5 and new_medio_char:
                characters += 1
                new_medio_char = False
            elif cuenta <= 5 and not new_medio_char:
                new_medio_char = True
            cuenta = 0
            
        px_last = px_actual

    if new_medio_char: characters += 1
    return characters,lista_grosor
