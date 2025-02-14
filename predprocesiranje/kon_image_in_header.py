from PIL import Image

def process_image_to_header(image_path, output_header, array_name):
    # Otvaranje slike
    img = Image.open(image_path)
    width, height = img.size
    
    # Konverzija u RGB (da bi se sacuvale sve tri vrednosti)
    img = img.convert("RGB")  # 'RGB' znaci da ce se sacuvati sve tri komponente (R, G, B)
    
    # Generisanje .h fajla
    with open(output_header, "w") as f:
        f.write(f"#ifndef {array_name.upper()}_H\n")
        f.write(f"#define {array_name.upper()}_H\n\n")
        
        f.write(f"#define WIDTH {width}\n")
        f.write(f"#define HEIGHT {height}\n\n")
        
        f.write(f"const unsigned char {array_name}[{width * height * 3}] __attribute__((section(\"seg_sdram1\"))) = {{\n")
        
        for y in range(height):
            for x in range(width):
                     #Izvlacenje RGB vrijednosti sa dati piksel
                r, g, b = img.getpixel((x, y)) 
                # Pisanje RGB vrednosti kao uzastopne vrednosti
                f.write(f"    {r}, {g}, {b},\n")
        
        f.write("};\n\n")
        f.write("#endif\n")
    
    print(f"Header fajl generisan: {output_header}")


# Primer koriscenja
image_pathA_jpg = "C:/Users/Administrator/Desktop/projekat SDOS/cces_projekat_novi/Debug/p30a.jpg"
image_pathB_jpg = "C:/Users/Administrator/Desktop/projekat SDOS/cces_projekat_novi/Debug/p30b.jpg"
output_headerA_jpg = "C:/Users/Administrator/Desktop/projekat SDOS/cces_projekat_novi/Debug/ulazna_slikaA_jpg.h"
output_headerB_jpg = "C:/Users/Administrator/Desktop/projekat SDOS/cces_projekat_novi/Debug/ulazna_slikaB_jpg.h"

image_pathA_bmp = "C:/Users/Administrator/Desktop/projekat SDOS/cces_projekat_novi/Debug/testna_slika6a.bmp"
image_pathB_bmp = "C:/Users/Administrator/Desktop/projekat SDOS/cces_projekat_novi/Debug/testna_slika6b.bmp"
output_headerA_bmp = "C:/Users/Administrator/Desktop/projekat SDOS/cces_projekat_novi/Debug/ulazna_slikaA_bmp.h"
output_headerB_bmp = "C:/Users/Administrator/Desktop/projekat SDOS/cces_projekat_novi/Debug/ulazna_slikaB_bmp.h"


process_image_to_header(image_pathA_jpg, output_headerA_jpg, "ulazna_slikaA_jpg")
process_image_to_header(image_pathB_jpg, output_headerB_jpg, "ulazna_slikaB_jpg")



process_image_to_header(image_pathA_bmp, output_headerA_bmp, "ulazna_slikaA_bmp")
process_image_to_header(image_pathB_bmp, output_headerB_bmp, "ulazna_slikaB_bmp")
