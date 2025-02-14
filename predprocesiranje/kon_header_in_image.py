from PIL import Image

def convert_h_to_rgb(input_file, output_file):
    # Otvaranje .h fajl i citajnje podataka
    with open(input_file, 'r') as file:
        lines = file.readlines()

    # Izvvašenje dimenzija slike iz .h fajla
    width = int([line for line in lines if line.startswith("#define WIDTH")][0].split()[2])
    height = int([line for line in lines if line.startswith("#define HEIGHT")][0].split()[2])

    # Provalazenje pixel vrednosti koje su u trećem delu fajla
    pixel_values = []
    reading_pixels = False

    for line in lines:
        if line.strip().startswith("const unsigned char"):  # Pocetak pixel podataka
            reading_pixels = True
            continue
        if reading_pixels:
            # Ako smo vec u delu sa pikselima, citamo vrednosti
            line_values = line.strip().strip(',').split(',')
            for val in line_values:
                if val.strip() and val.strip() != '};':  # Ignorisi "};" i prazne vrednosti
                    try:
                        pixel_values.append(int(val.strip()))
                    except ValueError:
                        pass  # Ako nije broj, preskoci

        if line.strip() == "};":
            break

    # Proveri da li broj piksela odgovara dimenzijama slike
    expected_pixel_count = width * height
    actual_pixel_count = len(pixel_values)

    if actual_pixel_count != expected_pixel_count:
        print(f"Greška: Broj piksela ({actual_pixel_count}) ne odgovara dimenzijama slike ({expected_pixel_count})!")
        return

    # Kreiraj listu piksela u RGB formatu
    pixels = [(pixel_values[i], pixel_values[i], pixel_values[i]) for i in range(len(pixel_values))]

    # Kreiraj sliku iz liste piksela
    image = Image.new('RGB', (width, height))
    image.putdata(pixels)

    # Sacuvaj sliku za zeljenoj lokaciji 
    image.save(output_file)
    print(f"Slika je uspešno sačuvana kao {output_file}")

# Poziv funkcije sa fajlovima
input_file = "C:/Users/Administrator/Desktop/projekat SDOS/cces_projekat_novi/Debug/izlazna_slika.h"
output_file = "C:/Users/Administrator/Desktop/projekat SDOS/cces_projekat_novi/Debug/slika_izlaza1.bmp"
convert_h_to_rgb(input_file, output_file)
