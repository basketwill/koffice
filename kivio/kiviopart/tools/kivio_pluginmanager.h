/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2003 Peter Simonsson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KIVIO_PLUGINMANAGER_H
#define KIVIO_PLUGINMANAGER_H

#include <qobject.h>
#include "kivio_mousetool.h"

namespace Kivio {
  /**
  This class manages the plugins*/
  class PluginManager : public QObject {
    Q_OBJECT
    public:
      PluginManager(KivioView* parent, const char* name = 0);
      ~PluginManager();

      void delegateEvent(QEvent* e);
            
      /** Returns the tool that is in use. */
      Kivio::MouseTool* activeTool();
      /** Returns the default tool. */
      Kivio::MouseTool* defaultTool();
    
    public slots:
      /** Makes the default tool active. */
      void activateDefaultTool();
      /** Makes @param tool active. */
      void activate(Kivio::MouseTool* tool);
      /** Make @param tool the default. */
      void setDefaultTool(Kivio::MouseTool* tool);
    
    private:
      Kivio::MouseTool* m_activeTool;
      Kivio::MouseTool* m_defaultTool;
  };
};

#endif
