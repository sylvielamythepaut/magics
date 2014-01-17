# importing Magics module

from Magics.macro import *


# Example reference

ref = 'bufr'

# Setting of the output file name

output = output(output_formats=['ps'],
                output_name_first_page_number='off',
                output_name=ref)

# Setting the coordinates of the geographical area

europe = mmap(
    subpage_upper_right_longitude=60.,
    subpage_map_projection='cylindrical',
    subpage_lower_left_longitude=-20.,
    subpage_lower_left_latitude=30.,
    subpage_upper_right_latitude=60.,
    )

# Coastlines setting

coast = mcoast(map_grid='on', map_grid_colour='tan',
               map_coastline_colour='tan',
               )

# Import andthe  data

obs = mobs(obs_input_file_name='synop.bufr')




title = \
    mtext(text_lines=["<font size='1'>Observation Plotting [Synop] </font>"
          , '',
          ], text_justification='left', text_font_size=0.5,
          text_colour='charcoal')



# To the plot

print "plot"
plot( output,  europe, obs, coast, )
tofortran(ref, output,  europe, obs, coast, )

