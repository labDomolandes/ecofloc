# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at

#   http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

import dash
from dash import dcc, html
#import dash_bootstrap_components as dbc


from floc_daemon import floc_daemon_layout, register_floc_daemon_callbacks
from system_monitor import system_monitor_layout, register_system_monitor_callbacks
from power_plotter import power_plotter_layout, register_power_plotter_callbacks
# import energy_plotter



#from nic_graph import register_callbacks as register_nic_callbacks, nic_graph_tab_layout

###################################################################################
##      THEMES
###################################################################################


#app = dash.Dash(__name__, external_stylesheets=[dbc.themes.SOLAR])
#app = dash.Dash(external_stylesheets=[dbc.themes.FLATLY])
#app = dash.Dash(__name__, external_stylesheets=[dbc.themes.BOOTSTRAP, dbc.icons.FONT_AWESOME])



#app = dash.Dash(__name__, external_stylesheets=[dbc.themes.MORPH, dbc.icons.FONT_AWESOME])



app = dash.Dash(__name__) # I finally use the the CSS in the assets folder (considered by default)


# BS = "https://bootswatch.com/5/morph/bootstrap.css"
# BS = "https://bootswatch.com/5/minty/bootstrap.rtl.css"
# app = dash.Dash(external_stylesheets=[BS])


###################################################################################
##      MAIN APP LAYOUT
###################################################################################


#Do I really need classes here? Dash objects are centralized
# energy_plotter = energy_plotter.EnergyPlotter(app) #Object to be refactored...maybe 
# MERGED IN A SINGLE PLOTTER


# USING CLASSES FOR CSS STYLE 


app.layout = html.Div([


    # Image and Title Section
    html.Div([
        html.Img(src="assets/FLOC.png", style={'width': '6%', 'margin-right': '10px', 'margin-left': '80px'}),
        html.Div([
            html.H1("FLOC: Be aware of your software's energy consumption", style={'textAlign': 'left'})
        ], style={'flex-grow': 1})
    ], style={'display': 'flex', 'align-items': 'center', 'justify-content': 'start', 'padding': '10px 0'}),

    # Tabs for different application modules
    dcc.Tabs(id="tabs", className='dcc_tabs', children=[
        dcc.Tab(
            label='1) SELECT PROCESS OR DATA ACTIVITY',
            className='dcc_tab',
            selected_className='dcc_tab--selected',
            children=[
                system_monitor_layout()
            ]
        ),
        dcc.Tab(
            label='2) LAUNCH FLOC DAEMON',
            className='dcc_tab',
            selected_className='dcc_tab--selected',
            children=[
                floc_daemon_layout()
            ]
        ),
        dcc.Tab(
            label='3) PLOT CURRENT POWER AND CUMULATED ENERGY',
            className='dcc_tab',
            selected_className='dcc_tab--selected',
            children=[
                power_plotter_layout()
            ]
        ),
        # dcc.Tab(
        #     label='4) PLOT ENERGY',
        #     className='dcc_tab',
        #     selected_className='dcc_tab--selected',
        #     children=[
        #         energy_plotter.energy_plotter_layout()
        #     ]
        # )
    ]),

    #  html.Div([
    #     html.Div([
    #         html.Img(src="assets/domolandes.png", style={'width': '50%', 'display': 'block', 'margin-left': 'auto', 'margin-right': 'auto', 'margin-top': '20px'})
    #     ], style={'width': '40%', 'display': 'inline-block', 'vertical-align': 'top', 'text-align': 'center'}),
        
    #     # New column for the additional images
    #     html.Div([
    #         html.Img(src="assets/UT1.png", style={'width': '40%', 'display': 'block', 'margin-left': 'auto', 'margin-right': 'auto', 'margin-top': '10px'}),
    #         html.Img(src="assets/UPPA.png", style={'width': '40%', 'display': 'block', 'margin-left': 'auto', 'margin-right': 'auto', 'margin-top': '10px'})
    #     ], style={'width': '20%', 'display': 'inline-block', 'vertical-align': 'top', 'text-align': 'center'})
    # ], className='header', style={'display': 'flex', 'justify-content': 'center', 'align-items': 'center'})

    html.Div([
        # First image
        html.Img(src="assets/domolandes.png", style={'width': '20%', 'margin': '20px'}),
        
        # Second image
        html.Img(src="assets/UT1.png", style={'width': '10%', 'margin': '20px'}),
        
        # Third image
        html.Img(src="assets/UPPA.png", style={'width': '20%', 'margin': '20px'})
    ], className='header', style={'display': 'flex', 'justify-content': 'center', 'align-items': 'center', 'gap': '10px'})


])


# app.layout = html.Div([

#     # THE HEADER STUFF
#     html.Div([
#         html.Img(src="assets/domolandes.png", style={'width': '20%', 'display': 'block', 'margin-left': 'auto', 'margin-right': 'auto', 'margin-top': '20px'})
#     ],className='header'),
#         html.Div([
#         html.Img(src="assets/FLOC.png", style={'width': '2%', 'margin-right': '10px','margin-left': '80px'}),  
#         html.Div([
#             html.H1("FLOC: Be aware of your software's energy consumption", style={'textAlign': 'left'})
#         ], style={'flex-grow': 1})
#     ], style={'display': 'flex', 'align-items': 'center', 'justify-content': 'start', 'padding': '10px 0'}),  


#     # THE OPTION TABS
#     dcc.Tabs(id="tabs", children=[
#         dcc.Tab(label='1) SYSTEM MONITOR', children=[
#             system_monitor_layout()  # Layout from the system monitor module
#         ]),
#         dcc.Tab(label='2) FLOC DAEMON LAUNCHER', children=[
#             floc_daemon_layout()  # Layout for the first tab
#         ]),
#         dcc.Tab(label='3) PLOT POWER', children=[
#             power_plotter_layout()  # Layout for the second tab from nic_graph.py
#         ]),
#         dcc.Tab(label='4) PLOT ENERGY', children=[
#             energy_plotter.energy_plotter_layout()  # Layout for the second tab from nic_graph.py
#         ]),
#     ])
# ])





# Callback registrations remain the same


register_floc_daemon_callbacks(app)
register_system_monitor_callbacks(app) 
register_power_plotter_callbacks(app)
# energy_plotter.register_energy_plotter_callbacks()

if __name__ == '__main__':
    app.run_server(debug=True)

